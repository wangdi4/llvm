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
    common_clang.h
    options.h
    binary_result.h
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
    options_compile.cpp
    options_link.cpp
)

if (WIN32)
    list( APPEND TARGET_SOURCE_FILES
            dllmain_win32.cpp
        )
else()
    list( APPEND TARGET_SOURCE_FILES
            dllmain_linux.cpp
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
