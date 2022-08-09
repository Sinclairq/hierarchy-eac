#pragma once
#include "stdafx.hpp"

namespace utils
{
    PIMAGE_NT_HEADERS const get_nt_headers( const uint64_t image_base );
    bool memcpy_safe( void *dest, void *src, uint32_t len );
    bool create_hook( uint8_t *address, void *target, void **p_original );
    void kprint( const char *msg, ... );
}
