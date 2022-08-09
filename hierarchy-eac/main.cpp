#include "stdafx.hpp"

eac_instance_t *g_eac_instance = nullptr;

long driver_entry( PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path )
{
    if ( !driver_object || !registry_path )
    {
        //
        // Driver is not meant to be manual mapped, add support for PsSetLoadImageNotify first...
        //

        return STATUS_UNSUCCESSFUL;
    }

    g_eac_instance = decltype( g_eac_instance )( ExAllocatePool( NonPagedPoolNx, sizeof( *g_eac_instance ) ) );
    driver_object->DriverUnload = []( PDRIVER_OBJECT object ) { PsRemoveLoadImageNotifyRoutine( vm::image_callback ); };

    return PsSetLoadImageNotifyRoutine( vm::image_callback );
}
