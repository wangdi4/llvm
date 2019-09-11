set(TARGET_NAME metadataapi)

add_library(
  ${TARGET_NAME}
  INTERFACE
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

target_include_directories(
  ${TARGET_NAME}
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}
)
