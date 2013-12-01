use_rtti(FALSE)

set(TARGET_NAME LLVMInterpreterPluggable)

include_directories(
  ${BACKEND_ROOT_DIR}/name_mangling
)

add_library(${TARGET_NAME}
  OpenCLCompilationFlags.h
  OpenCLCompilationFlags.cpp
  InterpreterPluggable.h
  InterpreterPluggable.cpp
  WorkGroupBuiltinsNames.h
  WorkGroupBuiltinsNames.cpp
  InterpreterPlugIn.h
)
#During the compilation of the LLVMInterpreter the Interpreter.h file is copying to the build directory 
#(and then to the install one - but this is irrelevant for current problem). 
#This file is then used by IntepreterPluggable build. So we need to ensure the LLVMInterpreter is running first
add_dependencies(${TARGET_NAME} LLVMInterpreter)