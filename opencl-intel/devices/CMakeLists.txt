# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system library directories
include(../cmake_utils/embed.rpath.txt)

# Add OpenMP on top TBB library
if( INCLUDE_MKL )
    message(STATUS "Including MKL library" )
	if (WIN32)
		set (ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
		if (NOT DEFINED ICC_WIN32_PATH)
			set (ICC_WIN32_PATH $ENV{ICPP_COMPILER13})
			if (NOT DEFINED ICC_WIN32_PATH)
				message (FATAL_ERROR "ICC 12 or 13 are not installed in the system")
			endif ()
		endif ()
		set( MKL_DIR "${ICC_WIN32_PATH}/mkl" )
		set( ICC_DIR "${ICC_WIN32_PATH}/compiler")
	else(WIN32)
                set( ICC_DIR "/opt/intel/composerxe")
                set( MKL_DIR "${ICC_DIR}/mkl" )
	endif(WIN32)
    add_definitions( -D__INCLUDE_MKL__ )

    # [Protex IP] Do not use omp2tbb.
    #if( USE_OMP2TBB )
    #message(STATUS "Including OMP2TBB library" )
    #add_definitions(-D__OMP2TBB__)
    #add_subdirectory( common/omp2tbb )
    #endif()

    if (ADDR EQUAL 64)
        set ( ICC_ARCH_DIR intel64 )
        add_definitions(-DMKL_ILP64 )
    else(ADDR EQUAL 64)
        set( ICC_ARCH_DIR ia32 )
    endif(ADDR EQUAL 64)

    add_subdirectory( common/mkl_kernels )
endif( INCLUDE_MKL)

add_subdirectory( common/builtin_kernels)
add_subdirectory( cpu_device )
add_subdirectory( device_commands )

# [LLVM 3.6 UPGRADE] Disable Linux warning about unused function
# this warning occur when processing externals/itt/include/ittnotify.h
# since it contain __itt_id __itt_id_make(void*, long long unsigned int)
# function that is not used in every build target.
if (NOT WIN32)
     set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
     set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
     # disable warnings inherited from external componennts
     add_definitions( "-Wno-unused-function" )
     add_definitions( "-Wno-attributes" )
endif(NOT WIN32)

if ( INCLUDE_MIC_DEVICE )
    add_subdirectory( mic_device )
endif ( INCLUDE_MIC_DEVICE )

option(INCLUDE_ISP_DEVICE "Include ISP device Into Build"  OFF)
if ( INCLUDE_ISP_DEVICE )
    add_subdirectory( isp_device )
endif( INCLUDE_ISP_DEVICE )
