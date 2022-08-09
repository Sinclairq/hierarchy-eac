#pragma once
#include "stdafx.hpp"

class eac_instance_t
{
private:
    void *image_base = 0;
    void *image_copy = 0;

    uint32_t image_size = 0;

    PIMAGE_NT_HEADERS nt_headers = 0;

public:
    uint8_t *get_image_base();
    uint8_t *get_raw_image();
    uint32_t get_img_size();

    PIMAGE_NT_HEADERS get_nt_headers();

    bool mem_in_bounds( uint8_t *address );
    bool mem_in_scn( uint8_t *address, uint32_t scn_type );

    void init( void *base, uint32_t size );
};
