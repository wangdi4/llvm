set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_RTTI 1)

if(WIN32)
  set(CLANG_BIN_PATH "${LLVM_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR}/clang")
else(WIN32)
  set(CLANG_BIN_PATH "${LLVM_TOOLS_BINARY_DIR}/clang")
endif(WIN32)

set(CLANG_INCLUDE_PATH "${BACKEND_ROOT_DIR}/clang_headers/")

add_definitions(
  -D CLANG_INCLUDE_PATH=${CLANG_INCLUDE_PATH}
  -D CLANG_BIN_PATH=${CLANG_BIN_PATH}
)

add_tablegen(
  ocl-tblgen
  cl_builtins
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/validations/OCLBuilder/
  ${CMAKE_SOURCE_DIR}/backend/validations/Common/
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
  ${LLVM_MAIN_SRC_DIR}/include
)

target_link_libraries(ocl-tblgen
  LLVMSupport
  LLVMCore
  LLVMBitReader
  LLVMAsmParser
)

add_dependencies(ocl-tblgen clang)
