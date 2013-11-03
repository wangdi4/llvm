cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

set (TARGET_NAME PlugInNEAT)
#
# Settings for Linux compilation
#
if(NOT WIN32)
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
endif(NOT WIN32)

add_library(${TARGET_NAME}
  PlugInNEAT.h
  PlugInNEAT.cpp
  )
include_directories (
  ../Interpreter
  ../InterpreterPluggable
  ${BACKEND_ROOT_DIR}/validations/OCLBuiltins
  ${BACKEND_ROOT_DIR}/../cl_api
  ${BACKEND_ROOT_DIR}/validations/DataManager
  ${BACKEND_ROOT_DIR}/validations/Common
  ${BACKEND_ROOT_DIR}/validations/NEAT
  ${BACKEND_ROOT_DIR}/validations/REFALU
  ${BACKEND_ROOT_DIR}/validations/ImathLibd
  ${BACKEND_ROOT_DIR}/name_mangling
  )

#During the compilation of the LLVMInterpreter the Interpreter.h file is copying to the build directory 
#(and then to the install one - but this is irrelevant for current problem). 
#This file is then used by IntepreterPluggable build. So we need to ensure the LLVMInterpreter is running first

add_dependencies(${TARGET_NAME} LLVMInterpreter)
