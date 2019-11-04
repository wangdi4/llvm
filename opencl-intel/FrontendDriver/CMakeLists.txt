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
# align with the definition in ocl-clang.
math(EXPR CCLANG_MAJOR_VERSION "${LLVM_VERSION_MAJOR} - 1")
set( CCLANG_LIBRARY_VERSION "${PRODUCTVER_MAJOR}.${CCLANG_MAJOR_VERSION}.${PRODUCTVER_MINOR}" )

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
set( INCLUDE_DIRS
            ${LLVM_INCLUDE_DIRS}
            ${CCLANG_DEV_INCLUDE_DIRS}
            ${CL_API_HEADERS}
            ${OCL_SOURCE_DIR}/backend/name_mangling
            ${OCL_SOURCE_DIR}/backend/plugin_manager
            ${OCL_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${OCL_SOURCE_DIR}/backend/dynamic_lib
            ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include
            ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
            ${OCL_SOURCE_DIR}/utils/cache_binary_handler
            ${OCL_SOURCE_DIR}/utils/CLElfLib
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
            ${OCL_BINARY_DIR}
            ${CMAKE_BINARY_DIR}/include
            )

if (OPENCL_INTREE_BUILD)
  if (LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR)
    list(APPEND INCLUDE_DIRS ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/include)
    add_definitions(-DOPENCL_INTREE_BUILD=1)
  endif()
endif()

if (WIN32)
    set(COMMON_CLANG_LIB ${LLVM_LIBRARY_DIRS}/common_clang${BUILD_PLATFORM}.lib)
else (WIN32)
    set(COMMON_CLANG_LIB ${LLVM_LIBRARY_DIR}/libcommon_clang${BUILD_PLATFORM}.so.${CCLANG_LIBRARY_VERSION})
    set(COMMON_CLANG_SYMLINK ${LLVM_LIBRARY_DIR}/libcommon_clang${BUILD_PLATFORM}.so)
endif (WIN32)

add_subdirectory(header_linker)

set(TARGET_SOURCES
    clang_device_info.h
    Compile.cpp
    Compile.h
    FrontendDriver.cpp
    FrontendDriver.h
    FrontendResultImpl.h
    GetKernelArgInfo.cpp
    GetKernelArgInfo.h
    Link.cpp
    Link.h
    ParseSPIRV.cpp
    ParseSPIRV.h
    SPIRMaterializer.cpp
    SPIRMaterializer.h
    SPIRVMaterializer.cpp
    SPIRVMaterializer.h
)

if (WIN32)
    list( APPEND TARGET_SOURCES fe_driver_main_win32.cpp )
else (WIN32)
    list( APPEND TARGET_SOURCES fe_driver_main_linux.cpp )
endif (WIN32)

add_subdirectory(pre_release_header)

add_opencl_library (${TARGET_NAME} SHARED
    ${TARGET_SOURCES}
    $<TARGET_OBJECTS:pre_release_header>
    INCLUDE_DIRS
      ${INCLUDE_DIRS}

    COMPONENTS
      cl_logger

    LINK_LIBS
      ${COMMON_CLANG_LIB}
      OclPluginManager
      CacheBinaryHandler
      CLElfLib
      name_mangle
      dynamic_load
      cl_sys_utils
      # LLVM LIBS
      # Doing so must be done with caution as cclang export all the symbols
      # on Linux (CORC-159). So one can never know which LLVM symbol (this or from cclang) would
      # be actually used for every particular call to LLVM.
      LLVMBitWriter
      LLVMCore
      LLVMLinker
      LLVMOption
      LLVMSupport
      LLVMSPIRVLib

    RC_TEMPLATE DEFAULT
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} common_clang${BUILD_PLATFORM})
endif()

if (WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:common_clang${BUILD_PLATFORM}.dll")
endif (WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${FRONTEND_FOLDER_NAME} )

if (WIN32)
    set (COMMON_CLANG_FILES ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}.dll)
else (WIN32)
    set (COMMON_CLANG_FILES ${COMMON_CLANG_LIB} ${COMMON_CLANG_SYMLINK})
endif (WIN32)
if (WIN32)
    if (INSTALL_PDBS)
        list (APPEND ${COMMON_CLANG_FILES} ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}.pdb)
    endif (INSTALL_PDBS)
    list (APPEND ${COMMON_CLANG_FILES} ${LLVM_BINARY_DIR}/common_clang${BUILD_PLATFORM}_stripped.pdb)
endif (WIN32)

SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} FrontendDriver.map )

if (NOT OPENCL_INTREE_BUILD)
  copy_to (${COMMON_CLANG_FILES} DESTINATION lib/${OUTPUT_ARCH_SUFF})
elseif (WIN32)
  add_custom_command( TARGET ${TARGET_NAME}
                      POST_BUILD
                      COMMAND  ${CMAKE_COMMAND} -E copy ${COMMON_CLANG_FILES}  ${OCL_OUTPUT_LIBRARY_DIR})
endif()
install_to (${COMMON_CLANG_FILES} DESTINATION lib/${OUTPUT_ARCH_SUFF} COMPONENT ocl-common_clang)
