use_eh(TRUE)
use_rtti(TRUE) 

message( STATUS "TableGen: CMAKE_CXX_COMPILER:" ${CMAKE_CXX_COMPILER})

if( NOT DEFINED CLANG_BIN_PATH)
	if(WIN32)
	  set(CLANG_BIN_PATH "${LLVM_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR}/clang")
	elseif(APPLE)
      find_program(CLANG_BIN_PATH openclc PATHS OPENCL_LIBRARIES DOC "OpenCL clang executable")
	else()
	  set(CLANG_BIN_PATH "${LLVM_TOOLS_BINARY_DIR}/clang")
	endif()
endif()

if( APPLE )
	set(CLANG_PARAMS "-cc1 -emit-llvm")
	set(CLANG_INCLUDE_PATH ".")
else()
	set(CLANG_PARAMS "-cc1 -emit-llvm -include opencl_.h -opencl-builtins")
	set(CLANG_INCLUDE_PATH "${BACKEND_ROOT_DIR}/clang_headers/")
endif()

add_definitions(
  -D CLANG_INCLUDE_PATH=${CLANG_INCLUDE_PATH}
  -D CLANG_PARAMS=${CLANG_PARAMS}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
)

if(APPLE)
	link_directories(
		${LLVM_LIBRARY_DIR}
	)
endif()

add_executable(
  ocl-tblgen
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  VectorizerTableGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
  ConversionParser.cpp
  ClangUtils.cpp
)

include_directories(
  #${CMAKE_SOURCE_DIR}/backend/validations/OCLBuilder/
  #${CMAKE_SOURCE_DIR}/backend/validations/Common/
  ${CMAKE_SOURCE_DIR}/backend/name_mangling/
  #${CMAKE_SOURCE_DIR}/cl_api
  ${OPENCL_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
  #${LLVM_MAIN_SRC_DIR}/include
)

target_link_libraries(ocl-tblgen
  name_mangle
  ${LLVM_MODULE_LIBS}
)
