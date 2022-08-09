#include "utils.hpp"

namespace utils
{
    PIMAGE_NT_HEADERS const get_nt_headers( const uint64_t image_base )
    {
        if ( !image_base )
            return nullptr;

        const PIMAGE_DOS_HEADER dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( image_base );
        if ( dos_header->e_magic != IMAGE_DOS_SIGNATURE )
            return nullptr;

        const PIMAGE_NT_HEADERS nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS >( image_base + dos_header->e_lfanew );
        if ( nt_headers->Signature != IMAGE_NT_SIGNATURE )
            return nullptr;

        return nt_headers;
    }

    bool memcpy_safe( void *dest, void *src, uint32_t len )
    {
        //
        // Too lazy to handle 0x1000+ writes, because they require a for loop
        // and we don't even need them here
        // physical pages are not guaranteed to be contiguous...
        //

        if ( len > PAGE_SIZE )
            return false;

        if ( !dest || !src )
            return false;

        if ( !MmIsAddressValid( dest ) || !MmIsAddressValid( src ) )
            return false;

        auto dst = static_cast< uint8_t * >( dest );
        auto source = static_cast< uint8_t * >( src );
        if ( &dst[ len ] > &static_cast< uint8_t * >( PAGE_ALIGN( dst ) )[ PAGE_SIZE ] )
        {
            //
            // We cannot write into a single buffer due to page boundary issues...
            //

            while ( len )
            {

                const auto phys_addr = MmGetPhysicalAddress( dst );
                if ( !phys_addr.QuadPart )
                    return false;

                const auto next_page = &static_cast< uint8_t * >( PAGE_ALIGN( dst ) )[ PAGE_SIZE ];
                const auto map_len = min( len, next_page - dst );
                const auto map_dest = MmMapIoSpace( phys_addr, map_len, MmNonCached );
                if ( !map_dest )
                    return false;

                memcpy( map_dest, source, map_len );
                MmUnmapIoSpace( map_dest, map_len );

                len -= map_len;
                dst += map_len;
                source += map_len;
            }

            return true;
        }

        const auto phys_addr = MmGetPhysicalAddress( dest );
        if ( !phys_addr.QuadPart )
            return false;

        const auto dest_map = MmMapIoSpace( phys_addr, len, MmNonCached );
        if ( !dest_map )
            return false;

        memcpy( dest_map, src, len );
        MmUnmapIoSpace( dest_map, len );

        return true;
    }

    bool create_hook( uint8_t *address, void *target, void **p_original )
    {
        if ( !address || !target )
            return false;

        hde64s block;
        uint8_t instr_len = 0;
        while ( 14 > instr_len )
            instr_len += hde64_disasm( &address[ instr_len ], &block );

        uint8_t rel64_jmp[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        const auto p_trampoline =
            static_cast< uint8_t * >( ExAllocatePool( NonPagedPool, sizeof( rel64_jmp ) + instr_len ) );

        if ( !p_trampoline )
            return false;

        auto return_to = &address[ instr_len ];

        //
        // TODO: Support RIP-relative instructions when copying original instructions....
        //

        memcpy( p_trampoline, &address[ 0 ], instr_len );
        memcpy( &p_trampoline[ instr_len ], rel64_jmp, sizeof( rel64_jmp ) );
        memcpy( &p_trampoline[ instr_len + 6 ], &return_to, sizeof( void * ) );

        if ( p_original )
            *p_original = p_trampoline;

        memcpy( &rel64_jmp[ 6 ], &target, sizeof( void * ) );
        utils::memcpy_safe( address, rel64_jmp, sizeof( rel64_jmp ) );

        return true;
    }

    void kprint( const char *msg, ... )
    {
        va_list args;
        va_start( args, msg );

        vDbgPrintExWithPrefix( "", DPFLTR_IHVVIDEO_ID, 0, msg, args );
    }

} // namespace utils
