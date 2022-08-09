#include "eac.hpp"

uint8_t *eac_instance_t::get_image_base()
{
	return static_cast<uint8_t *>( image_base );
}

uint32_t eac_instance_t::get_img_size()
{
	return image_size;
}

uint8_t *eac_instance_t::get_raw_image()
{
	return static_cast<uint8_t *>( image_copy );
}

bool eac_instance_t::mem_in_bounds( uint8_t *address )
{
	if ( address >= get_image_base() && address <= &get_image_base()[get_img_size()] )
		return true;

	return false;
}

bool eac_instance_t::mem_in_scn( uint8_t *address, uint32_t scn_type )
{
	const auto rva = address - static_cast<uint8_t *>( image_base );
	const auto scn_headers = IMAGE_FIRST_SECTION( nt_headers );

	for ( auto i = 0U; i < nt_headers->FileHeader.NumberOfSections; ++i )
	{
		const auto *c_section = &scn_headers[i];
		if ( !( c_section->Characteristics & scn_type ) )
			continue;

		if ( rva >= c_section->VirtualAddress &&
			rva <= static_cast<uint64_t>( c_section->VirtualAddress ) + c_section->Misc.VirtualSize )
			return true;
	}

	return false;
}

PIMAGE_NT_HEADERS eac_instance_t::get_nt_headers()
{
	return this->nt_headers;
}

void eac_instance_t::init( void *base, uint32_t size )
{
	image_base = base;
	image_size = size;

	if ( !image_base || !image_size )
		return;

	image_copy = ExAllocatePool( NonPagedPoolNx, image_size );
	if ( !image_copy )
		return;

	memcpy( image_copy, image_base, image_size );
	nt_headers = utils::get_nt_headers( reinterpret_cast<uint64_t>( image_base ) );
}
