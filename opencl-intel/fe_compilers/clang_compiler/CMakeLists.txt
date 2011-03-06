#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME clang_compiler )

set(LLVM_NO_RTTI      1)
set(SHARED_LIBRARY TRUE)

if (NOT WIN32)
    remove_definitions( -Wno-strict-aliasing -Wno-parentheses -Wunused-variable )
    add_definitions( -Werror )

	# gcc 4.1 complains about missing virtual dtor for interface classes. this is bad and wrong, we should supress it.
	add_definitions("-Wno-non-virtual-dtor")
endif (NOT WIN32)


# [DK]:
# under Windows this project should be built with Intel C++
# Compiler is replaced by the running batch script
# Release mode is confogured to build with IPO, but because some of
# linked-in libs are not built with Intel Compiler, IPO fails on this
# project because of different formats. Disable IPO here.
IF(MSVC)
  string(REPLACE /GL "" CMAKE_C_FLAGS_RELEASE   ${CMAKE_C_FLAGS_RELEASE})
  string(REPLACE /GL "" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
ENDIF(MSVC)

set( EXTERNAL_HEADERS_LOCAL_SUBDIR export/clang_include)
file(GLOB EXTERNAL_HEADERS ${EXTERNAL_HEADERS_LOCAL_SUBDIR}/*.h)
set_source_files_properties(${EXTERNAL_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)

if (WIN32)
    set(DLLMAIN dllmain_win32.cpp)
	set(RESOURCCES OpenCL.rc clang_compiler.rc)
else (WIN32)
    set(DLLMAIN dllmain_linux.cpp)
	set(RESOURCCES )
endif(WIN32)

link_directories( ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )

# add a target named ${TARGET_NAME}
add_clang_library( ${TARGET_NAME}
		${RESOURCCES}
        clang_compiler.cpp
        clang_driver.cpp
        ${DLLMAIN}
        stdafx.cpp
        ${EXTERNAL_HEADERS}
        )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${FE_COMPILER_FOLDER_NAME} )
set(FILES_TO_COPY ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )

if (WIN32)
	get_target_property( OUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY )
else (WIN32)
	get_target_property( OUT_DIR ${TARGET_NAME} LIBRARY_OUTPUT_DIRECTORY )
endif(WIN32)

COPY_FILES_POST_BUILD_EX( ${TARGET_NAME}
                              ${OUT_DIR}/${CMAKE_CFG_INTDIR}
                              ${OCL_INSTALL_DIR}
                              ${FILES_TO_COPY} )

set( CLANG_LINK_COMPONENTS
        clangFrontend
        clangDriver
        clangCodeGen
        clangSema
        clangChecker
        clangAnalysis
        clangRewrite
        clangAST
        clangParse
        clangLex
        clangBasic
        )

set(LLVM_LINK_COMPONENTS
        LLVMAnalysis
        LLVMArchive
        LLVMAsmParser
        LLVMAsmPrinter
        LLVMBitReader
        LLVMBitWriter
        LLVMCodeGen
        LLVMCore
        LLVMExecutionEngine
        LLVMInstCombine
        LLVMInstrumentation
        LLVMInterpreter
        LLVMipa
        LLVMipo
        LLVMJIT
        LLVMLinker
        LLVMMC
        LLVMMCParser
        LLVMScalarOpts
        LLVMSelectionDAG
        LLVMSupport
        LLVMSystem
        LLVMTarget
        LLVMTransformUtils
        LLVMX86AsmParser
        LLVMX86AsmPrinter
        LLVMX86CodeGen
        LLVMX86Disassembler
        LLVMX86Info
        )

if (WIN32)
    set( LINK_LIBS   PowrProf.lib )
endif(WIN32)

							
target_link_libraries( ${TARGET_NAME}
                            ${CLANG_LINK_COMPONENTS}
                            ${LLVM_LINK_COMPONENTS}
                            cl_logger
                            cl_sys_utils
                            task_executor
							${LLVM_LINK_COMPONENTS}
                            ${LINK_LIBS} )

# [DK]:
# under Windows this project should be built with Intel C++ with extentions enabed
# Compiler is replaced by the running batch script, extention enabling should be done here
IF(MSVC)
  get_target_property(NON_ANSI_COMPILE_FLAGS ${TARGET_NAME} COMPILE_FLAGS)
  string(REPLACE /Za
    "" NON_ANSI_COMPILE_FLAGS
    ${NON_ANSI_COMPILE_FLAGS})
  set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS ${NON_ANSI_COMPILE_FLAGS})
ENDIF(MSVC)

# ---------------------------------------------------------------------------------------------------
#    External headers copying and PCH compilation
# ---------------------------------------------------------------------------------------------------

# copy required CLANG compiler Intel OpenCL headers to the output dir into fe_include subdir
file(GLOB EXTERNAL_HEADERS_NO_PATH
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}
     ${EXTERNAL_HEADERS_LOCAL_SUBDIR}/*.h)

# this function generates variable COPIED_FILES
set( INSTALLED_HEADERS_PATH ${OCL_INSTALL_DIR}/fe_include )
GEN_RULES_FOR_COPY_FILES( ${INSTALLED_HEADERS_PATH}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}
                          ${EXTERNAL_HEADERS_NO_PATH} )

set_source_files_properties(${COPIED_FILES} PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE)

# if headers were copied - regenerate precompiled headers also. Put dependecies on COPIED_FILES
# to ensure that opencl_.h files was already copied before. Put also dependecy on the clang_compiler
# to ensure that clang_compiler build and opencl_.h copying will not occur in parallel. The reason
# for possible parallelism is that CMake may copy header files during any of targets in the same directory
# and PCH generation and opencl_.h copying are configured as such.
#
# now generate precompiled headers
# NOTE!!!!!
#   1. Must reference to the opencl_.h with the full path in the installation area under OCL_INSTALL_DIR!!!!
#   2. -isysroot must point to the OCL_INSTALL_DIR
#   3. output must be in the OCL_INSTALL_DIR

if (WIN32)
	set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
	set( COMPILER_OPTIONS -cc1
						  -x cl
						  -emit-pch 
						  -I ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}
						  )
	set( REDIRECTION_OPTION < ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}/opencl_.h )
else (WIN32)
	set( OPENCL_PCH_OUT_DIR ${OCL_INSTALL_DIR} )
	set( COMPILER_OPTIONS -cc1
						  -triple amd64	
						  -x cl
						  -I ${INSTALLED_HEADERS_PATH}
						  -emit-pch )

	set( REDIRECTION_OPTION < ${INSTALLED_HEADERS_PATH}/opencl_.h )
endif (WIN32)

add_custom_command( OUTPUT ${OPENCL_PCH_OUT_DIR}/opencl_.pch
                    COMMAND clang ${COMPILER_OPTIONS} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}/opencl_.h
                    DEPENDS ${COPIED_FILES} ${EXTERNAL_HEADERS}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    VERBATIM
                   )
#We dont understand why we need it - Should ask Dima about...
#add_custom_command( OUTPUT ${OCL_INSTALL_DIR}/opencl_opt_.pch
#                    COMMAND clang ${COMPILER_OPTIONS} -O3 -o ${OCL_INSTALL_DIR}/opencl_opt_.pch
#                    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERNAL_HEADERS_LOCAL_SUBDIR}/opencl_.h
#                    DEPENDS ${COPIED_FILES} ${EXTERNAL_HEADERS}
#                    COMMENT "Generate optimized Intel OpenCL precompiled headers"
#                    VERBATIM
#                   )

#set_source_files_properties(${OPENCL_PCH_OUT_DIR}/opencl_.pch ${OCL_INSTALL_DIR}/opencl_opt_.pch PROPERTIES GENERATED TRUE)
set_source_files_properties(${OPENCL_PCH_OUT_DIR}/opencl_.pch PROPERTIES GENERATED TRUE)

#add_custom_target( clang_precompiled_headers ALL
#                    DEPENDS ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${OCL_INSTALL_DIR}/opencl_opt_.pch 
#                    COMMENT "Copy Intel OpenCL headers"
#                    SOURCES ${EXTERNAL_HEADERS}
#                    VERBATIM)
add_custom_target( clang_precompiled_headers ALL
                    DEPENDS ${OPENCL_PCH_OUT_DIR}/opencl_.pch 
                    COMMENT "Copy Intel OpenCL headers"
                    SOURCES ${EXTERNAL_HEADERS}
                    VERBATIM)

set_target_properties( clang_precompiled_headers PROPERTIES FOLDER ${FE_COMPILER_FOLDER_NAME} )

if (WIN32)
	#Because On Windows we need the pch files before the linkage of the clang_compiler
	add_dependencies( ${TARGET_NAME} clang_precompiled_headers)
else (WIN32)
	#Stay with the old style
	add_dependencies( clang_precompiled_headers ${TARGET_NAME} )
endif (WIN32)

