cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME name_mangle)

set(ANTLR_COMMAND
  java -cp ${CMAKE_CURRENT_SOURCE_DIR}/antlruntime/lib/antlr.jar antlr.Tool
)

project (${TARGET_NAME})

set(NAME_MANGLE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT APPLE)
    add_subdirectory(unittests)
endif (NOT APPLE)
add_subdirectory(antlruntime)

add_custom_command(
  OUTPUT
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleParser.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleLexer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/DemangleLexer.hpp
  COMMAND ${ANTLR_COMMAND}
  -o ${CMAKE_CURRENT_SOURCE_DIR}/gen ${CMAKE_CURRENT_SOURCE_DIR}/demangler.g
  DEPENDS demangler.g
  COMMENT "running ${ANTLR_COMMAND} on ${CMAKE_CURRENT_SOURCE_DIR}/demangler.g"
)

add_library(
  ${TARGET_NAME}
  STATIC
  Type.cpp
  Demangler.cpp
  Mangler.cpp
  FunctionDescriptor.cpp
  TypeCast.cpp
  Utils.cpp
  gen/DemangleLexer.cpp
  gen/DemangleParser.cpp
)

include_directories(
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${CMAKE_CURRENT_SOURCE_DIR}/antlruntime/include
  ${CMAKE_CURRENT_SOURCE_DIR}/gen
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  ${TARGET_NAME}
  antlr
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
