set(LLVM_REQUIRES_EH 1)
set(LLVM_REQUIRES_RTTI 1)

link_directories(${CMAKE_SOURCE_DIR}/backend/name_mangling/runtime/lib)

add_tablegen(
  ocl-tblgen
  cl_builtins
  OclBuiltinEmitter.cpp
  OclBuiltinsHeaderGen.cpp
  TableGen.cpp
  CodeFormatter.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
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
  name_mangle
  OCLBuilder
  antlr
)
