add_subdirectory( cpu_device )
add_subdirectory( device_commands )

if ( INCLUDE_MIC_DEVICE )
    add_subdirectory( mic_device )
endif ( INCLUDE_MIC_DEVICE )

option(INCLUDE_ISP_DEVICE "Include ISP device Into Build"  OFF)
if ( INCLUDE_ISP_DEVICE )
    add_subdirectory( isp_device )
endif( INCLUDE_ISP_DEVICE )
