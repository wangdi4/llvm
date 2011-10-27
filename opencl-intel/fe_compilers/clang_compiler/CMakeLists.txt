#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME clang_compiler )

include_directories( AFTER
            ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/llvm/tools/clang/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../../backend/llvm/include
            ${CMAKE_CURRENT_BINARY_DIR}/../../backend/llvm/tools/clang/include
            ${CMAKE_CURRENT_BINARY_DIR}/../../backend/llvm/include
            )
			
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

link_directories( 
			${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR}
			${CMAKE_CURRENT_BINARY_DIR}/../../backend/llvm/lib/${INSTALL_SUBDIR}
			)

set(FILES_TO_COPY 
		${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} 
		)
			
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

add_dependencies( ${TARGET_NAME} clang)


#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} 
									  task_executor
									  clangFrontendTool
									  clangAnalysis
									  clangAST
									  clangBasic
									  clangChecker
									  clangCodeGen
									  clangDriver
									  clangFrontend
									  clangFrontendTool
									  clangIndex
									  clangLex
									  clangParse
									  clangRewrite
									  clangSema
									  clangSerialization
									  libclang
									  LLVMMCParser
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
									  LLVMJIT
									  LLVMLinker									  
									  LLVMMCDisassembler
									  LLVMScalarOpts									  
									  LLVMSupport
									  LLVMSystem									  
									  LLVMTransformUtils
									  LLVMX86AsmParser									  
									  LLVMX86CodeGen
									  LLVMX86Disassembler
									  LLVMX86Info
									  LLVMipo
									  LLVMX86AsmPrinter
									  LLVMSelectionDAG
									  LLVMTarget
									  LLVMMC
									  LLVMipa)

set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( COMPILER_OPTIONS -cc1
					  -x cl
					  -emit-pch 
					  -I ${CMAKE_CURRENT_SOURCE_DIR}/export/clang_include
					  )
set( REDIRECTION_OPTION < ${CMAKE_CURRENT_SOURCE_DIR}/export/clang_include/opencl_.h )
	
add_custom_command( TARGET clang_compiler
					PRE_BUILD
                    COMMAND clang ${COMPILER_OPTIONS} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    VERBATIM
                   )


install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_.pch DESTINATION bin)
install(DIRECTORY export/clang_include/ DESTINATION include/fe_include PATTERN *.h PATTERN .svn EXCLUDE )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler )

add_llvm_definitions( -D__STDC_LIMIT_MACROS )
add_llvm_definitions( -D__STDC_CONSTANT_MACROS )
