# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#
use_eh(TRUE)
use_rtti(FALSE)

# set that name of the main output file as a target name
set( TARGET_NAME clang_compiler${BUILD_PLATFORM} )

#
# Source code
#
set(TARGET_INCLUDE_FILES
    clang_compiler.h
    clang_device_info.h
    pch_mgr.h
    resource.h
    exceptions.h
    stdafx.h
    targetver.h
)

set(TARGET_SOURCE_FILES
    pch_mgr.cpp
    stdafx.cpp
    clang_compiler.rc
    OpenCL.rc
)

if (WIN32)
    list( APPEND TARGET_SOURCE_FILES
            dllmain_win32.cpp
        )
else()
    list( APPEND TARGET_SOURCE_FILES
            dllmain_linux.cpp
            ${CMAKE_BINARY_DIR}/bin/opencl_pch.cpp
            ${CMAKE_BINARY_DIR}/bin/opencl20_pch.cpp
        )
endif()

if (USE_COMMON_CLANG)
    #
    # TblGen the options include file
    #
    if(CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID)
        set (TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
    else()
        set (TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
    endif()

    set (COMMON_CLANG_DIR ${CMAKE_SOURCE_DIR}/fe_compilers/common_clang)
    set (COMPILE_OPTIONS_TD  ${COMMON_CLANG_DIR}/opencl_clang_options.td)
    set (COMPILE_OPTIONS_INC ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/opencl_clang_options.inc)
    set (LINK_OPTIONS_TD  ${COMMON_CLANG_DIR}/opencl_link_options.td)
    set (LINK_OPTIONS_INC ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/opencl_link_options.inc)
    set (TBLGEN ${TOOL_BINARY_DIR}/clang-tblgen)

    set (CUSTOM_COMMAND_LLVM_INCLUDE_DIRS)
    string( REPLACE  \${CMAKE_CFG_INTDIR} ${CMAKE_CFG_INTDIR} TEMP_LLVM_INCLUDE_DIR "${LLVM_INCLUDE_DIRS}" )
    foreach( LLVM_INCLUDE_DIR_ITEM ${TEMP_LLVM_INCLUDE_DIR} )
      set(CUSTOM_COMMAND_LLVM_INCLUDE_DIRS ${CUSTOM_COMMAND_LLVM_INCLUDE_DIRS} -I${LLVM_INCLUDE_DIR_ITEM})
    endforeach( LLVM_INCLUDE_DIR_ITEM)

    add_custom_command(
        OUTPUT ${COMPILE_OPTIONS_INC}
        COMMAND ${TBLGEN} --gen-opt-parser-defs ${CUSTOM_COMMAND_LLVM_INCLUDE_DIRS} -o ${COMPILE_OPTIONS_INC} ${COMPILE_OPTIONS_TD}
        DEPENDS ${TBLGEN} ${COMPILE_OPTIONS_TD}
        COMMENT "Generating ${COMPILE_OPTIONS_INC}: ${TBLGEN} --gen-opt-parser-defs ${CUSTOM_COMMAND_LLVM_INCLUDE_DIRS} -o ${COMPILE_OPTIONS_INC} ${COMPILE_OPTIONS_TD}"
        )

    add_custom_command(
        OUTPUT ${LINK_OPTIONS_INC}
        COMMAND ${TBLGEN} --gen-opt-parser-defs ${CUSTOM_COMMAND_LLVM_INCLUDE_DIRS} -o ${LINK_OPTIONS_INC} ${LINK_OPTIONS_TD}
        DEPENDS ${TBLGEN} ${LINK_OPTIONS_TD}
        COMMENT "Generating ${LINK_OPTIONS_INC}: ${TBLGEN} --gen-opt-parser-defs ${CUSTOM_COMMAND_LLVM_INCLUDE_DIRS} -o ${LINK_OPTIONS_INC} ${LINK_OPTIONS_TD}"
        )
        
    list( APPEND TARGET_SOURCE_FILES
        ${COMMON_CLANG_DIR}/link.cpp
        ${COMMON_CLANG_DIR}/getkernelarginfo.cpp
        ${COMMON_CLANG_DIR}/options.cpp
        ${COMMON_CLANG_DIR}/options_link.cpp
        ${COMMON_CLANG_DIR}/options_compile.cpp
        clang_compiler_cc.cpp
        clang_driver_cc.cpp
        translation_chain.cpp
        translation_controller.cpp
        translator.cpp
        )

    list( APPEND TARGET_INCLUDE_FILES
        clang_driver_cc.h 
        tc_common.h
        translation_chain.h
        translation_controller.h
        TranslationBlock.h
        translator.h
        ${COMMON_CLANG_DIR}/options.h
        ${COMPILE_OPTIONS_TD}
        ${LINK_OPTIONS_TD}
        ${COMPILE_OPTIONS_INC}
        ${LINK_OPTIONS_INC}
        )
else()
    list( APPEND TARGET_SOURCE_FILES
        clang_compiler.cpp
        clang_driver.cpp
        )
    list( APPEND TARGET_INCLUDE_FILES
        clang_driver.h
        )
endif()

#
# Compilation flags
#
set( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -D OCL_DEV_BACKEND_PLUGINS -D OCLFRONTEND_PLUGINS")

if(USE_COMMON_CLANG)
    add_definitions( -DUSE_COMMON_CLANG )
    add_definitions( -DCOMMON_CLANG_EXPORTS )
    include_directories( AFTER ${LLVM_INCLUDE_DIRS})
    link_directories( ${LLVM_LIBRARY_DIRS})
endif()
#
# Include directories
#
include_directories( AFTER
            ${CMAKE_SOURCE_DIR}/fe_compilers/common_clang 
            ${CMAKE_SOURCE_DIR}/backend/metadata_api
            ${CMAKE_SOURCE_DIR}/backend/plugin_manager
            ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
            ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
            ${CMAKE_SOURCE_DIR}/utils/cache_binary_handler
            ${CMAKE_SOURCE_DIR}/utils/CLElfLib
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
            ${OCL_BINARY_DIR}
            ${CMAKE_BINARY_DIR}/include
            )

if (WIN32)
    include_directories( AFTER
        ../common/windows/headers
    )
else()
    include_directories( AFTER
        ../common/linux/headers
    )
endif()

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
                        metadataapi
                        dynamic_load
                        cl_logger
                        cl_sys_utils
)

if (USE_COMMON_CLANG)
target_link_libraries( ${TARGET_NAME}
                       clangDriver
                       clangBasic
                       ${LLVM_MODULE_LIBS}
)
else()
target_link_libraries( ${TARGET_NAME}
                        common_clang${BUILD_PLATFORM}
)
endif()    

if (NOT ANDROID AND NOT WIN32)
    list(APPEND LINK_LIBS pthread)
endif ()

set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

if (BUILD_X64)
  set( TRIPLE -triple spir64-unknown-unknown )
else()
  set( TRIPLE -triple spir-unknown-unknown )
endif()

set( CLANG_OPTIONS -cc1
                   -x cl
                   -O0
                   -emit-pch
                   -I ${OCL_CLANG_HEADERS}
                   -D __IMAGE2D_DEPTH__
                   -D __WRITE_IMAGE3D__
                   )

set( CLANG_OPTIONS_OPENCL_12
                    ${CLANG_OPTIONS}
                      )

set( CLANG_OPTIONS_OPENCL_20
                    ${CLANG_OPTIONS}
                    -cl-std=CL2.0
                    -D__OPENCL_C_VERSION__=200
                      )

set( REDIRECTION_OPTION < ${OCL_CLANG_HEADERS}/opencl_.h
)

if( USE_COMMON_CLANG )
    string(REPLACE "\${CMAKE_CFG_INTDIR}" "\$<CONFIGURATION>" TOOL_BINARY_DIR ${COMMON_CLANG_PATH})
else()
    if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
        set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
    else()
        set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
    endif()
endif()

set( CLANG ${TOOL_BINARY_DIR}/clang )

if (WIN32)
    # The custom actions for generating the opencl_.pch/opencl20_.pch for windows and linux are almost equal
    # but using only the latter currently breaks the USE_COMMON_CLANG option (which is soon to be deprecated).
    # TODO: unify the following 2 sets of custom commands and put them outside of the if(WIN32) once 
    # USE_COMMON_CLANG is deprecated.
    add_custom_command( TARGET ${TARGET_NAME}
                        PRE_BUILD
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_12}  ${TRIPLE} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                        DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                        VERBATIM
    )

    add_custom_command( TARGET ${TARGET_NAME}
                        PRE_BUILD
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_20} ${TRIPLE} -o ${OPENCL_PCH_OUT_DIR}/opencl20_.pch ${REDIRECTION_OPTION}
                        COMMENT "Generate non-optimized Intel OpenCL20 precompiled headers"
                        DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                        VERBATIM
    )
else()
    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/opencl_.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_12}  ${TRIPLE} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                        DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                        VERBATIM
    )

    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/opencl20_.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_20} ${TRIPLE} -o ${OPENCL_PCH_OUT_DIR}/opencl20_.pch ${REDIRECTION_OPTION}
                        COMMENT "Generate non-optimized Intel OpenCL20 precompiled headers"
                        DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                        VERBATIM
    )

    if(CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID)
        set (LINUX_RESOURCE_LINKER_EXE "${CMAKE_BINARY_DIR}/native/bin/linux_resource_linker")
    else()
        set (LINUX_RESOURCE_LINKER_EXE "${CMAKE_BINARY_DIR}/bin/linux_resource_linker")
    endif()

    add_custom_command (
        OUTPUT ${CMAKE_BINARY_DIR}/bin/opencl_pch.cpp
        DEPENDS ${LINUX_RESOURCE_LINKER_EXE} ${CLANG} ${OPENCL_PCH_OUT_DIR}/opencl_.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${CMAKE_BINARY_DIR}/bin/opencl_pch.cpp PCH_101
    )

    add_custom_command (
        OUTPUT ${CMAKE_BINARY_DIR}/bin/opencl20_pch.cpp
        DEPENDS ${LINUX_RESOURCE_LINKER_EXE} ${CLANG} ${OPENCL_PCH_OUT_DIR}/opencl20_.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/opencl20_.pch ${CMAKE_BINARY_DIR}/bin/opencl20_pch.cpp PCH_102
    )
endif()

add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_12} -triple i686-pc-linux -o ${OPENCL_PCH_OUT_DIR}/opencl_android_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                    VERBATIM
)

add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_20} -triple i686-pc-linux -o ${OPENCL_PCH_OUT_DIR}/opencl20_android_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL20 precompiled headers"
                    DEPENDS ${CLANG} ${OCL_CLANG_HEADERS}/opencl_.h
                    VERBATIM
)

install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_.pch DESTINATION bin)
install(FILES ${OPENCL_PCH_OUT_DIR}/opencl20_.pch DESTINATION bin)
install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_android_.pch DESTINATION bin)
install(FILES ${OPENCL_PCH_OUT_DIR}/opencl20_android_.pch DESTINATION bin)
install(DIRECTORY ${OCL_CLANG_HEADERS}/ DESTINATION include/fe_include PATTERN *.h EXCLUDE )
if( USE_COMMON_CLANG )
    string(REPLACE "\${CMAKE_CFG_INTDIR}" "\${BUILD_TYPE}" COMMON_CLANG_INSTALL_PATH ${COMMON_CLANG_PATH})
    install(FILES ${COMMON_CLANG_INSTALL_PATH}/igdfcl${ADDR}.dll 
            DESTINATION bin )
    install(FILES ${COMMON_CLANG_INSTALL_PATH}/igdfcl${ADDR}.pdb 
            DESTINATION bin )
endif()

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
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")
endif(WIN32)

