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
include(../cmake_utils/embed.rpath.txt)

#
# Source code
#
set(TARGET_INCLUDE_FILES
    clang_compiler.h
    clang_device_info.h
    clang_driver.h
    SPIRMaterializer.h
    resource.h
    stdafx.h
    targetver.h
)

set(TARGET_SOURCE_FILES
    clang_compiler.cpp
    clang_compiler.rc
    clang_driver.cpp
    SPIRMaterializer.cpp
    stdafx.cpp
)

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

add_library(${TARGET_NAME} SHARED
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)


target_link_libraries( ${TARGET_NAME}
                        OclPluginManager
                        CacheBinaryHandler
                        CLElfLib
                        name_mangle
                        dynamic_load
                        cl_logger${EXPERIMENTAL_POSTFIX}
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

if(WIN32)
    target_link_libraries( ${TARGET_NAME}
        ${LLVM_LIBRARY_DIRS}/common_clang${BUILD_PLATFORM}.lib
    )
else(WIN32)
    target_link_libraries( ${TARGET_NAME}
            ${LLVM_BINARY_DIR}/libcommon_clang${BUILD_PLATFORM}.so
    )
endif(WIN32)

if (NOT ANDROID AND NOT WIN32)
    list(APPEND LINK_LIBS pthread)
endif ()

install(DIRECTORY ${OCL_CLANG_HEADERS}/ DESTINATION include/fe_include PATTERN *.h EXCLUDE )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${FRONTEND_FOLDER_NAME} )

#
# Stripped PDB files
#
if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS    "${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE    "${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
    if (INSTALL_PDBS)
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)
    install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
else (WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} clang_compiler.map )
endif(WIN32)


if(WIN32)
    string(REPLACE "${CMAKE_CFG_INTDIR}" "\${BUILD_TYPE}" LLVM_BINARY_DIR_FOR_INSTALL ${LLVM_BINARY_DIR})
    install( FILES ${LLVM_BINARY_DIR_FOR_INSTALL}/common_clang${BUILD_PLATFORM}.dll DESTINATION bin )
    if(INSTALL_PDBS)
        install( FILES ${LLVM_BINARY_DIR_FOR_INSTALL}/common_clang${BUILD_PLATFORM}.pdb DESTINATION bin )
    endif(INSTALL_PDBS)
    install( FILES ${LLVM_BINARY_DIR_FOR_INSTALL}/common_clang${BUILD_PLATFORM}_stripped.pdb DESTINATION bin )
else(WIN32)
    install( FILES ${LLVM_BINARY_DIR}/libcommon_clang${BUILD_PLATFORM}.so DESTINATION bin )
endif(WIN32)
