#include "vm.hpp"

namespace vm
{
    void *g_sha1_update = nullptr;
    void *g_is_reloc = nullptr;

    void is_reloc( context_t *context )
    {

        const auto address = reinterpret_cast< uint8_t * >( context->rcx );
        const auto read_addr = context->rcx;

        if ( g_eac_instance->mem_in_bounds( address ) && g_eac_instance->mem_in_scn( address, IMAGE_SCN_MEM_EXECUTE ) )
        {
            const auto rebased = reinterpret_cast< uint64_t >( address - g_eac_instance->get_image_base() +
                                                               g_eac_instance->get_raw_image() );

            //
            // To 'spoof' all driver reads, we use IoGetStackLimits to
            // get the start-end address of the stack then proceed to
            // iterate through the entire stack looking for instances of the target
            // address then modifying it to our 'cleaned' address
            //
            // Moreover, spoofing registers are necessary to make sure the address
            // is no longer reachable from the current registers
            //

            uint64_t stack_low = 0, stack_high = 0;
            IoGetStackLimits( &stack_low, &stack_high );

            while ( stack_low < stack_high )
            {
                const auto current_offset = reinterpret_cast< uint64_t * >( stack_low );
                if ( *current_offset == read_addr )
                    *current_offset = rebased;

                stack_low += sizeof( uint64_t );
            }

            const auto ctx_array = reinterpret_cast< uint64_t * >( context );
            for ( auto i = 0UL; i < sizeof( *context ); i++ )
            {
                if ( ctx_array[ i ] == read_addr )
                    ctx_array[ i ] = rebased;
            }

            const auto rva = read_addr - reinterpret_cast< uint64_t >( g_eac_instance->get_image_base() );
            if ( rva == reinterpret_cast< uint64_t >( PAGE_ALIGN( rva ) ) )
            {
                //
                // Only log if EAC is reading the start of a page to prevent excess logging....
                //

                utils::kprint( "is_reloc(tid: %i) reading -> EasyAntiCheat.sys+0x%x\n", CURRENT_THREAD_ID, rva );
            }
        }

        //
        // this isn't a traditional hook; we must call the original and update the result register
        // with the return value, otherwise we can expect undefined behavior...
        //

        context->rax = reinterpret_cast< bool ( * )( uint64_t ) >( g_is_reloc )( read_addr );
    }

    void *sha1_create_hash( void *sha1_ctx, uint8_t *address, uint64_t len )
    {

        if ( g_eac_instance->mem_in_bounds( address ) )
        {
            if ( g_eac_instance->mem_in_scn( address, IMAGE_SCN_MEM_EXECUTE ) )
            {
                {
                    utils::kprint( "Spoofing SHA1 read from(tid: %i) rva => 0x%x, len => 0x%04x!\n", CURRENT_THREAD_ID,
                                   address - g_eac_instance->get_image_base(), len );

                    address = ( address - g_eac_instance->get_image_base() ) + g_eac_instance->get_raw_image();
                }
            }
        }

        return reinterpret_cast< decltype( &sha1_create_hash ) >( g_sha1_update )( sha1_ctx, address, len );
    }

    void image_callback( PUNICODE_STRING image_name, HANDLE proc_id, PIMAGE_INFO image_info )
    {
        if ( image_name && image_name->Buffer && image_name->Length )
        {
            if ( !wcsstr( image_name->Buffer, L"EasyAntiCheat.sys" ) )
                return;

            utils::kprint( "EasyAntiCheat.sys loaded at base => 0x%p, size => 0x%x\n", image_info->ImageBase,
                           image_info->ImageSize );

            //
            // Initialize the copy of EasyAntiCheat.sys used for spoofing reads....
            //

            g_eac_instance->init( image_info->ImageBase, image_info->ImageSize );
            utils::kprint( "EasyAntiCheat.sys copy loaded at base => 0x%p\n", g_eac_instance->get_raw_image() );

            utils::create_hook( EAC_SHA1_CREATE_HASH( image_info->ImageBase ), &sha1_create_hash, &g_sha1_update );
            utils::create_hook( EAC_IS_RELOC( image_info->ImageBase ), &vm_is_reloc, &g_is_reloc );
        }
    }
} // namespace vm
