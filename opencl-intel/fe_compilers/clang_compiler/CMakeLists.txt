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
            ${CMAKE_SOURCE_DIR}/backend/plugin_manager
            ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
            )

			
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

if (WIN32)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2008/${OCL_SYS_DEPENDENT_SUBDIR} )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( 
			${TBB_LIB_DIR}
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

if (NOT WIN32) 
  list(APPEND LINK_LIBS
        LLVMCBackend
        LLVMCBackendInfo
        LLVMCppBackend
        LLVMCppBackendInfo
      )
endif (NOT WIN32)



target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} 
									  task_executor
									  clangFrontendTool
									  clangAnalysis
									  clangAST
									  clangBasic
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
									  LLVMMCJIT
									  LLVMScalarOpts									  
									  LLVMSupport									  
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
									  LLVMipa
                    OclPluginManager)

set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( COMPILER_OPTIONS -cc1
					  -x cl
					  -O0
					  -emit-pch 
					  -I ${OCL_CLANG_HEADERS}
					  )
set( REDIRECTION_OPTION < ${OCL_CLANG_HEADERS}/opencl_.h
)
	
add_custom_command( TARGET clang_compiler
					PRE_BUILD
                    COMMAND clang ${COMPILER_OPTIONS} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    VERBATIM
                   )


install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_.pch DESTINATION bin)
install(DIRECTORY ${OCL_CLANG_HEADERS}/ DESTINATION include/fe_include PATTERN *.h PATTERN .svn EXCLUDE )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler )

if (WIN32)
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008 
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS	"${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")	    
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.		
	    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG	"${LINK_FLAGS_DEBUG} /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE	"${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
endif(WIN32)

add_llvm_definitions( -D__STDC_LIMIT_MACROS )
add_llvm_definitions( -D__STDC_CONSTANT_MACROS )
add_definitions(-DOCLBACKEND_PLUGINS)
add_definitions(-DOCLFRONTEND_PLUGINS)
