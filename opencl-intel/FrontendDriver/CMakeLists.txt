# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files

use_eh(TRUE)
use_rtti(FALSE)

if( NOT DEFINED LLVM_PATH_FE )
    message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM libraries location using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

# set that name of the main output file as a target name
set( TARGET_NAME clang_compiler${BUILD_PLATFORM} )

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard system library directories
include(embed.rpath)

#
# Compilation flags
#
set( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -D OCL_DEV_BACKEND_PLUGINS -D OCLFRONTEND_PLUGINS")
add_definitions( ${LLVM_DEFINITIONS} )
if (NOT WIN32)
  set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
endif(NOT WIN32)

#
# Include directories
#
include_directories( AFTER
            ${LLVM_INCLUDE_DIRS}
            ${LLVM_INCLUDE_DIRS}/cclang
            ${CMAKE_SOURCE_DIR}/backend/name_mangling
            ${CMAKE_SOURCE_DIR}/backend/plugin_manager
            ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
            ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
            ${CMAKE_SOURCE_DIR}/utils/cache_binary_handler
            ${CMAKE_SOURCE_DIR}/utils/CLElfLib
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
            ${OCL_BINARY_DIR}
            ${CMAKE_BINARY_DIR}/include
            ${CL_API_HEADERS}
            ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include
            )

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
endif ()

add_opencl_library (${TARGET_NAME} SHARED
    SPIRMaterializer.cpp
    SPIRMaterializer.h
    FrontendDriver.cpp
    FrontendDriver.h
    FrontendDriver.rc
    clang_device_info.h
    clang_driver.cpp
    clang_driver.h
    resource.h
)
if (WIN32)
    set(COMMON_CLANG_LIB ${LLVM_LIBRARY_DIRS}/common_clang${BUILD_PLATFORM}.lib)
else (WIN32)
    set(COMMON_CLANG_LIB ${LLVM_LIBRARY_DIR}/libcommon_clang${BUILD_PLATFORM}.so)
endif (WIN32)

target_link_libraries(${TARGET_NAME}
    ${COMMON_CLANG_LIB}
    OclPluginManager
    CacheBinaryHandler
    CLElfLib
    name_mangle
    dynamic_load
    cl_logger${OPENCL_BINARIES_POSTFIX}
    cl_sys_utils
    # LLVM LIBS
    # Doing so must be done with caution as cclang export all the symbols
    # on Linux (CORC-159). So one can never know which LLVM symbol (this or from cclang) would
    # be actually used for every particular call to LLVM.
    LLVMBitWriter
    LLVMCore
    LLVMSupport
    LLVMSPIRVLib
)

if (NOT WIN32)
    list(APPEND LINK_LIBS pthread)
endif ()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${FRONTEND_FOLDER_NAME} )

if (WIN32)
    set (COMMON_CLANG_FILES ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}.dll)
else (WIN32)
    set (COMMON_CLANG_FILES ${COMMON_CLANG_LIB})
endif (WIN32)
if (WIN32)
    if (INSTALL_PDBS)
        list (APPEND ${COMMON_CLANG_FILES} ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}.pdb)
    endif (INSTALL_PDBS)
    list (APPEND ${COMMON_CLANG_FILES} ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}_stripped.pdb)
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} FrontendDriver.map )
endif (WIN32)
copy_to (${COMMON_CLANG_FILES} DESTINATION lib)
