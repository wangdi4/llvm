add_llvm_library(LLVMInterpreter
  Execution.cpp
  ExternalFunctions.cpp
  Interpreter.cpp
  )

if( LLVM_BUILD_VERIFICATION_LIB )
  add_definitions(-DVALIDATION_ON)
  include_directories(${CMAKE_SOURCE_DIR}/../validations/OCLBuiltins)
  target_link_libraries(LLVMInterpreter OCLBuiltins)
  add_dependencies(LLVMInterpreter OCLBuiltins)
endif(LLVM_BUILD_VERIFICATION_LIB)
