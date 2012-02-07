project(Validation)
cmake_minimum_required(VERSION 2.8)

# Variables for llvm libraries
set (VALIDATIONS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

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
add_subdirectory(InterpreterPluggable)
add_subdirectory(PlugInNEAT)
#add_subdirectory(Unittests)
