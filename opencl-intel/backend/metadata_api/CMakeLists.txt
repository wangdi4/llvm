cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME metadataapi)

add_library(
  ${TARGET_NAME}
  INTERFACE
)

target_include_directories(
  ${TARGET_NAME}
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}
)
