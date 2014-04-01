#
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
    clang_driver.h
    resource.h
    stdafx.h
    targetver.h
)

set(TARGET_SOURCE_FILES
    clang_compiler.cpp
    clang_driver.cpp
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
        )
endif()

if (USE_COMMON_CLANG)
    list( APPEND TARGET_SOURCE_FILES
            translation_chain.cpp
            translation_controller.cpp
            translator.cpp
        )
    list( APPEND TARGET_INCLUDE_FILES
            tc_common.h
            translation_chain.h
            translation_controller.h
            TranslationBlock.h
            translator.h
        )
endif()

#
# Compilation flags
#
set( CMAKE_CXX_FLAGS_DEBUG
  "${CMAKE_CXX_FLAGS_DEBUG} -D OCL_DEV_BACKEND_PLUGINS -D OCLFRONTEND_PLUGINS"
)

add_definitions( -D__STDC_LIMIT_MACROS )
add_definitions( -D__STDC_CONSTANT_MACROS )
if(USE_COMMON_CLANG)
    add_definitions( -DUSE_COMMON_CLANG )
endif()

#
# Include directories
#
include_directories( AFTER
            ${LLVM_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/backend/metadata_api
            ${CMAKE_SOURCE_DIR}/backend/plugin_manager
            ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
            ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
            ${CMAKE_SOURCE_DIR}/utils/cache_binary_handler
            ${CMAKE_SOURCE_DIR}/utils/CLElfLib
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

set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

link_directories( 
    ${TBB_LIB_DIR}
    ${LLVM_LIBRARY_DIR}
)

add_library(${TARGET_NAME} SHARED 
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

if (NOT INCLUDE_MIC_DEVICE)
if(NOT WIN32)
    # fix the case when LLVM is built with MIC support, but we want OCL without MIC support.
    if (${LLVM_COMPONENTS} MATCHES "(y86.*)")
        set(LLVM_MODULE_LIBS "-lLLVMY86Disassembler -lLLVMY86CodeGen -lLLVMMICModuleJIT -lLLVMY86AsmParser -lLLVMY86Desc -lLLVMY86Info -lLLVMY86AsmPrinter -lLLVMY86Utils ${LLVM_MODULE_LIBS} -lpcg -lsvml -lirc")
    endif()
else()
    # Try to find Y86 libraries on windows.
    # Release mode
    string(REPLACE "${CMAKE_CFG_INTDIR}" "Release" RELEASE_DIR ${LLVM_LIBRARY_DIR})
    find_library(IS_Y86_TARGET LLVMY86CodeGen ${RELEASE_DIR})
    if (IS_Y86_TARGET)
        target_link_libraries(${TARGET_NAME} optimized
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMMICModuleJIT
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            libpcg
            svml_dispmt
            libirc
            libmmt
            libdecimal
            )
    endif()
    # Debug mode
    string(REPLACE "${CMAKE_CFG_INTDIR}" "Debug" DEBUG_DIR ${LLVM_LIBRARY_DIR})
    find_library(IS_Y86_TARGET LLVMY86CodeGen ${DEBUG_DIR})
    if (IS_Y86_TARGET)
        target_link_libraries(${TARGET_NAME} debug
            LLVMY86AsmParser
            LLVMY86CodeGen
            LLVMMICModuleJIT
            LLVMY86Desc
            LLVMY86AsmPrinter
            LLVMY86Disassembler
            LLVMY86Info
            LLVMY86Utils
            libpcg
            svml_dispmt
            libirc
            libmmt
            libdecimal
            )
    endif()
endif()
endif (NOT INCLUDE_MIC_DEVICE)

target_link_libraries( ${TARGET_NAME} 
                           CacheBinaryHandler
                           CLElfLib
)

if (USE_COMMON_CLANG)
    target_link_libraries( ${TARGET_NAME} 
                           OclPluginManager
                           metadataapi
                           dynamic_load
                           cl_logger
                           cl_sys_utils
                           SPIRMetadataAdder
                           ${LLVM_MODULE_LIBS}
    )
else() 
    # The list of clang libraries is taken from clang makefile
    # (build/tools/clang/tools/driver/CMakeFiles/clang.dir/build.make)
    # All duplicate libraries are there on purpose 
    target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                          SPIRMetadataAdder
                                          cl_sys_utils
                                          cl_logger
#                                          LLVMCppBackendCodeGen
#                                          LLVMCppBackendInfo
                                          task_executor${BUILD_PLATFORM}
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
                                          OclPluginManager
                                          #CLElfLib
                                          metadataapi
                                          dynamic_load
                                          cl_sys_utils
                                          ${LLVM_MODULE_LIBS}
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

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
   set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()
    
set( CLANG ${TOOL_BINARY_DIR}/clang )

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
install(DIRECTORY ${OCL_CLANG_HEADERS}/ DESTINATION include/fe_include PATTERN *.h PATTERN .svn EXCLUDE )

if( USE_COMMON_CLANG )
    string(REPLACE "\${CMAKE_CFG_INTDIR}" "\${BUILD_TYPE}" COMMON_CLANG_INSTALL_PATH ${COMMON_CLANG_PATH})

    install(FILES ${COMMON_CLANG_INSTALL_PATH}/igdfcl${ADDR}.dll 
            DESTINATION bin 
            RENAME common_clang${ADDR}.dll)
    install(FILES ${COMMON_CLANG_INSTALL_PATH}/igdfcl${ADDR}.pdb 
            DESTINATION bin 
            RENAME common_clang${ADDR}.pdb)
    install(FILES ${COMMON_CLANG_INSTALL_PATH}/igdfcl${ADDR}.lib
            DESTINATION bin 
            RENAME common_clang${ADDR}.lib)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler )

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


