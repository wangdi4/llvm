if(NOT WIN32)
add_definitions(-msse4.2)
endif(NOT WIN32)

set(TARGET_NAME OclRecorder)

project(${TARGET_NAME})

cmake_minimum_required(VERSION 2.6.1)

if( CMAKE_SIZEOF_VOID_P EQUAL 4)
	# 32 bit
	set (PLATFORM 32)
else( CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64 bit
	set (PLATFORM 64)
endif( CMAKE_SIZEOF_VOID_P EQUAL 4)

set(OCL_RECORDER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCL_RECORDER_IMPORT_DIR ${BACKEND_ROOT_DIR}/../cl_api)
set(OCL_RECORDER_EXPORT_DIR ${OCL_RECORDER_DIR}/export)
set(OCL_RECORDER_LIB_DIR ${OCL_RECORDER_DIR}/lib/Win${PLATFORM})

# Variables for ocldevbackend project
set (OCL_BACKEND_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ocl_cpu_backend)
set (OCL_BACKEND_EXPORTS_DIR ${OCL_BACKEND_ROOT}/export)
set (OCL_BACKEND_IMPORTS_DIR ${OCL_BACKEND_ROOT}/../cl_api)

# Other dependency directories
set (OCL_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../external)
set (OCL_DATAMANAGER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../validations/DataManager)
set (OCL_NEAT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../validations/NEAT)

	
set (OCL_RECORDER_INCLUDE_FILES
	ocl_recorder.h
	stdafx.h
	targetver.h
  )
  
set (OCL_RECORDER_SOURCE_FILES
	dllmain.cpp
	ocl_recorder.cpp
	stdafx.cpp
  )

# Resources
if (WIN32)	
	set (OCL_RECORDER_RESOURCE_FILES
		ocl_recorder.rc
	)
else (WIN32)
	set (OCL_RECORDER_RESOURCE_FILES
	)
endif (WIN32)

# LLVM libs
set (LLVM_LIBS
	LLVMLinker
	LLVMX86AsmParser
	LLVMX86AsmPrinter
	LLVMX86CodeGen
	LLVMX86Disassembler
	LLVMX86Info
	LLVMTransformUtils
	LLVMCore
	LLVMTarget
	LLVMSystem
	LLVMExecutionEngine
	LLVMJIT
	LLVMInterpreter
	LLVMCodeGen
	LLVMBitReader
	LLVMBitWriter
	LLVMSupport
	LLVMAnalysis
	LLVMAsmPrinter
	LLVMInstCombine
	LLVMipa
	LLVMipo
	LLVMMC
	LLVMScalarOpts
	LLVMSelectionDAG
	LLVMVectorizer
	)
	
set (OCL_RECORDER_DEPS
	tinyxml_STL
	DataManager
	${LLVM_LIBS}
	) 	

# Add include directories
include_directories(${OCL_RECORDER_IMPORT_DIR} ${OCL_RECORDER_EXPORT_DIR} )
include_directories(${OCL_BACKEND_IMPORTS_DIR} ${OCL_BACKEND_EXPORTS_DIR} )
include_directories(${OCL_EXTERNAL_DIR}/tinyxml ${OCL_DATAMANAGER_DIR} ${OCL_NEAT_DIR})

# Add additional lib direcories
link_directories (${OCL_RECORDER_LIB_DIR}) 

# Create DLL
add_library (${TARGET_NAME} SHARED 
	${OCL_RECORDER_INCLUDE_FILES}
	${OCL_RECORDER_SOURCE_FILES}
	${OCL_RECORDER_RESOURCE_FILES}
	${ASM_ADD_TO_SOURCES_LIST}
	)
	
# Link with appropriate libs
target_link_libraries (${TARGET_NAME} ${OCL_RECORDER_DEPS}) 

# Define the dependancy projects
add_dependencies(${TARGET_NAME} ${OCL_RECORDER_DEPS})

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)


if(WIN32)
# Define the additional link flags	
set(CMAKE_SHARED_LINKER_FLAGS  /NODEFAULTLIB:LIBCMTD)
endif(WIN32)
