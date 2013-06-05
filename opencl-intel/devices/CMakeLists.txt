add_subdirectory( cpu_device )

if ( INCLUDE_MIC_DEVICE )
    add_subdirectory( mic_device )
endif ( INCLUDE_MIC_DEVICE )

option(INCLUDE_GEN_DEVICE "Include GEN device Into Build"  OFF)
if ( INCLUDE_GEN_DEVICE )
    add_subdirectory( gen_device )
endif( INCLUDE_GEN_DEVICE )
