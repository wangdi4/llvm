#
# Settings for Linux compilation
#
if (NOT WIN32)

# Compiler switches that CANNOT be modified during makefile generation
set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")

set (ADD_C_FLAGS         "${ADD_COMMON_C_FLAGS} -std=gnu99")
set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")

set (ADD_C_FLAGS_DEBUG   "-O0 -ggdb -D _DEBUG")
set (ADD_C_FLAGS_RELEASE "-O2 -g0 ")

# C switches
set( CMAKE_C_FLAGS         "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
set( CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

# C++ switches
set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

endif (NOT WIN32)

add_llvm_library(PlugInNEAT
  PlugInNEAT.cpp
  )
include_directories (
  ../Interpreter
  ../InterpreterPluggable
  ${CMAKE_SOURCE_DIR}/../validations/OCLBuiltins
  ${CMAKE_SOURCE_DIR}/../cl_api
  ${CMAKE_SOURCE_DIR}/../validations/DataManager
  ${CMAKE_SOURCE_DIR}/../validations/Common
  ${CMAKE_SOURCE_DIR}/../validations/NEAT
  ${CMAKE_SOURCE_DIR}/../validations/REFALU
  )

target_link_libraries(PlugInNEAT
  NEAT
  DataManager
  LLVMSupport
  OCLBuiltins
  )

add_dependencies( PlugInNEAT
  LLVMInterpreterPluggable
  DataManager
  LLVMSupport
  OCLBuiltins
  )
