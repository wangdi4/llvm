#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#
project( common_clang )
use_eh(TRUE)
use_rtti(FALSE)

# set that name of the main output file as a target name
set( TARGET_NAME common_clang${BUILD_PLATFORM} )

if (NOT WIN32)
    #
    # Usage
    #     SET_LINUX_EXPORTS_FILE( TARGET FILE_NAME )
    # This function provides special linkage options for OCL Back-End. These options prevents exposure of function symbols
    # externally (except functions which are explicitly listed in version script

    function ( SET_LINUX_EXPORTS_FILE TARGET FILE_NAME )
            get_target_property( SOURCE_FILES ${TARGET_NAME} SOURCES )
            list( GET SOURCE_FILES 0 FIRST_SOURCE )
            set_source_files_properties( ${FIRST_SOURCE} PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME} )
            set_property( TARGET ${TARGET_NAME} APPEND_STRING PROPERTY
                            LINK_FLAGS "-Wl,-Bsymbolic -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}" )
    endfunction ( SET_LINUX_EXPORTS_FILE )
endif()    

#
# TblGen the options include file
#
if(CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID)
    set (TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
    set (TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()

set (COMPILE_OPTIONS_TD  ${CMAKE_CURRENT_SOURCE_DIR}/opencl_clang_options.td)
set (COMPILE_OPTIONS_INC ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/opencl_clang_options.inc)
set (LINK_OPTIONS_TD  ${CMAKE_CURRENT_SOURCE_DIR}/opencl_link_options.td)
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

#
# Source code
#
set(TARGET_INCLUDE_FILES
    stdafx.h
    targetver.h
    exceptions.h
    common_clang.h
    options.h
    binary_result.h
    pch_mgr.h
    ${COMPILE_OPTIONS_TD}
    ${LINK_OPTIONS_TD}
    ${COMPILE_OPTIONS_INC}
    ${LINK_OPTIONS_INC}
)

set(TARGET_SOURCE_FILES
    stdafx.cpp
    common_clang.cpp
    link.cpp
    getkernelarginfo.cpp
    common_clang.rc
    options.cpp
    pch_mgr.cpp
    options_compile.cpp
    options_link.cpp
    OpenCL.rc
)

if (WIN32)
    list( APPEND TARGET_SOURCE_FILES
            dllmain_win32.cpp
        )
else()
    list( APPEND TARGET_SOURCE_FILES
            dllmain_linux.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/opencl_common_pch.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_pch.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_20_pch.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_12_pch.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/header_with_defs_cpu.h.cpp
        )
endif()

add_definitions( -D__STDC_LIMIT_MACROS )
add_definitions( -D__STDC_CONSTANT_MACROS )
add_definitions( -DCOMMON_CLANG_EXPORTS )

#
# Include directories
#
include_directories( AFTER
            ${OCL_BINARY_DIR}
            ${LLVM_INCLUDE_DIRS}
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
            ${CMAKE_SOURCE_DIR}/cl_api
            )

link_directories( 
    ${LLVM_LIBRARY_DIRS}
)

add_library(${TARGET_NAME} SHARED 
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (NOT WIN32)
    SET_LINUX_EXPORTS_FILE( ${TARGET_NAME} common_clang_linux_exports.txt )
endif (NOT WIN32)

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

# The list of clang libraries is taken from clang makefile
# (build/tools/clang/tools/driver/CMakeFiles/clang.dir/build.make)
# All duplicate libraries are there on purpose 
target_link_libraries( ${TARGET_NAME} 
                       LINK_PRIVATE
                       clangFrontendTool
                       clangAST
                       clangAnalysis
                       clangBasic
                       clangCodeGen
                       clangDriver
                       clangEdit
                       clangFrontend
                       clangLex
                       clangParse
                       clangEdit
                       clangARCMigrate
                       clangRewriteCore
                       clangRewriteFrontend
                       clangSema
                       clangSerialization
                       clangStaticAnalyzerFrontend
                       clangStaticAnalyzerCheckers
                       clangStaticAnalyzerCore
                       clangRewriteFrontend
                       clangFrontend
                       clangDriver
                       clangSerialization
                       clangRewriteCore
                       clangParse
                       clangSema
                       clangAnalysis
                       clangEdit
                       clangAST
                       clangLex
                       clangBasic
                       ${LLVM_MODULE_LIBS}
                       )

if (NOT ANDROID AND NOT WIN32)
    list(APPEND LINK_LIBS pthread)
endif ()

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

#
# Generate PCHs.
#

set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( CLANG_OPTIONS -cc1
                   -x cl
                   -fno-validate-pch
                   -O0
                   -emit-pch
                   -working-directory ${OPENCL_PCH_OUT_DIR}
                   -D __IMAGE2D_DEPTH__
                   -D __WRITE_IMAGE3D__
                   )

set( CLANG_OPTIONS_OPENCL_12
                    ${CLANG_OPTIONS}
                    -cl-std=CL1.2
                    -D __OPENCL_C_VERSION__=120
                    )

set( CLANG_OPTIONS_OPENCL_20
                    ${CLANG_OPTIONS}
                    -cl-std=CL2.0
                    -D __OPENCL_C_VERSION__=200
                    )

if (BUILD_X64)
    set( TRIPLE -triple spir64-unknown-unknown )
else()
    set( TRIPLE -triple spir-unknown-unknown )
endif()

set( CL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/cl_headers/ )
set( CLANG ${TOOL_BINARY_DIR}/clang )

set( REDIRECTION_OPTION_COMMON   < ${CL_HEADERS}/opencl_common.h           )
set( REDIRECTION_OPTION_CPU      < ${CL_HEADERS}/cpu/cpu_opencl_.h         )
set( REDIRECTION_OPTION_CPU_PLAT < ${CL_HEADERS}/cpu/cpu_opencl_plat_spec.h)

if (WIN32)
    # The custom actions for generating the opencl_.pch/opencl20_.pch for windows and linux are almost equal
    # but using only the latter currently breaks the USE_COMMON_CLANG option (which is soon to be deprecated).
    # TODO: unify the following 2 sets of custom commands and put them outside of the if(WIN32) once
    # USE_COMMON_CLANG is deprecated.
    add_custom_command( TARGET ${TARGET_NAME}
                        PRE_BUILD
                        COMMAND ${CLANG} ${CLANG_OPTIONS} -o ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch ${REDIRECTION_OPTION_COMMON}
                        COMMENT "Generate non-optimized Intel Op"
                        DEPENDS ${CLANG} ${CL_HEADERS}/opencl_common.h
                        VERBATIM
        )

    # Generate cpu specific file.
    add_custom_command( TARGET ${TARGET_NAME}
                        PRE_BUILD
                        COMMAND ${CLANG} ${CLANG_OPTIONS} -include-pch opencl_common.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch ${REDIRECTION_OPTION_CPU}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers. CPU_OPENCL_.PCH ."
                        DEPENDS ${CLANG} ${CL_HEADERS}/cpu_opencl_.h ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch
                        VERBATIM
    )

    #Generate two files for OpenCL version specific.
    add_custom_command( TARGET ${TARGET_NAME}
                        PRE_BUILD
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_12} ${TRIPLE} -include-pch cpu_opencl_.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_12.h.pch ${REDIRECTION_OPTION_CPU_PLAT}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                        DEPENDS ${CLANG} ${CL_HEADERS}/cpu_opencl_plat_spec.h ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
                        VERBATIM
     )

   add_custom_command( TARGET ${TARGET_NAME}
                       PRE_BUILD
                       COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_20} ${TRIPLE} -include-pch cpu_opencl_.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_20.h.pch ${REDIRECTION_OPTION_CPU_PLAT}
                       COMMENT "Generate non-optimized Intel OpenCL precompiled headers. CPU_OPENCL_CPU_PLAT ."
                       DEPENDS ${CLANG} ${CL_HEADERS}/cpu_opencl_plat_spec.h ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
                       VERBATIM
    )

else()
    # Generate common file.
    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS}  ${TRIPLE} -o ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch ${REDIRECTION_OPTION_COMMON}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers opencl_common.h.pch ."
                        DEPENDS ${CLANG} ${CL_HEADERS}/opencl_common.h
                        VERBATIM
    )

    # Generate cpu specific file.
    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS}  ${TRIPLE} -include-pch opencl_common.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch ${REDIRECTION_OPTION_CPU}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers cpu_opencl_.h.pch ."
                        DEPENDS ${CLANG} ${CL_HEADERS}/cpu/cpu_opencl_.h ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch
                        VERBATIM
    )

    #Generate two files for OpenCL version specific.
    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_12.h.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_12} ${TRIPLE} -include-pch cpu_opencl_.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_12.h.pch ${REDIRECTION_OPTION_CPU_PLAT}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers cpu_opencl_plat_spec_12.h.pch ."
                        DEPENDS ${CLANG} ${CL_HEADERS}/cpu/cpu_opencl_plat_spec.h ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
                        VERBATIM
    )

    add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_20.h.pch
                        COMMAND ${CLANG} ${CLANG_OPTIONS_OPENCL_20} ${TRIPLE} -include-pch cpu_opencl_.h.pch -o ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_20.h.pch ${REDIRECTION_OPTION_CPU_PLAT}
                        COMMENT "Generate non-optimized Intel OpenCL precompiled headers cpu_opencl_plat_spec_20.h.pch ."
                        DEPENDS ${CLANG} ${CL_HEADERS}/cpu/cpu_opencl_plat_spec.h ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
                        VERBATIM
    )

    if(CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID)
        set (LINUX_RESOURCE_LINKER_EXE "${CMAKE_BINARY_DIR}/native/bin/linux_resource_linker")
        set (RESOURCE_TARGET ConfigureNativeTargets)
    else()
        set (LINUX_RESOURCE_LINKER_EXE "${CMAKE_BINARY_DIR}/bin/linux_resource_linker")
        set (RESOURCE_TARGET linux_resource_linker)
    endif()

    # Convert PCHs and header_with_defs_cpu to array in C.
    add_custom_command (
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/opencl_common_pch.cpp
        DEPENDS ${RESOURCE_TARGET} ${CLANG} ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/opencl_common.h.pch ${CMAKE_CURRENT_BINARY_DIR}/opencl_common_pch.cpp PCH_COMMON
    )

    add_custom_command (
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_pch.cpp
        DEPENDS ${RESOURCE_TARGET} ${CLANG} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_.h.pch ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_pch.cpp PCH_CPU
    )

    add_custom_command (
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_20_pch.cpp
        DEPENDS ${RESOURCE_TARGET} ${CLANG} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_20.h.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_20.h.pch ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_20_pch.cpp PCH_CPU_SPEC_20
    )

    add_custom_command (
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_12_pch.cpp
        DEPENDS ${RESOURCE_TARGET} ${CLANG} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_12.h.pch
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${OPENCL_PCH_OUT_DIR}/cpu_opencl_plat_spec_12.h.pch ${CMAKE_CURRENT_BINARY_DIR}/cpu_opencl_plat_spec_12_pch.cpp PCH_CPU_SPEC_12
    )

    add_custom_command (
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/header_with_defs_cpu.h.cpp
        DEPENDS ${RESOURCE_TARGET} ${CLANG} ${CL_HEADERS}/cpu/header_with_defs_cpu.h
        COMMAND ${LINUX_RESOURCE_LINKER_EXE} ${CL_HEADERS}/cpu/header_with_defs_cpu.h ${CMAKE_CURRENT_BINARY_DIR}/header_with_defs_cpu.h.cpp PCH_DEFS_CPU
    )
endif()
