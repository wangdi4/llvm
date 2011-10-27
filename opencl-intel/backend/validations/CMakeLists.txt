project(Validation)
cmake_minimum_required(VERSION 2.6.1)

# Variables for llvm libraries
set (LLVM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../llvm)
set (LLVM_INCLUDE_DIR ${LLVM_ROOT}/include)

# LLVM-specific variables
# TODO: it's not good solution to define the same variables into 2 CMakeLists.txt. FIX IT!
set(LLVM_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/../build/Win32)

set (VALIDATIONS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../validations)

include_directories( ${LLVM_BINARY_DIR}/include )
include_directories( ${LLVM_INCLUDE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Common )

#set (MIC_ENABLE ON)

add_subdirectory(DataManager)
add_subdirectory(DataGenerator)
add_subdirectory(Comparator)
add_subdirectory(NEAT)
add_subdirectory(REFALU)
add_subdirectory(SATest)
add_subdirectory(NEATCheck)
add_subdirectory(OCLRecorder)
add_subdirectory(OCLBuiltins)
