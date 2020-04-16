set(TARGET_NAME AddNTAttr)

set(TARGET_INCLUDE_FILES
  AddNTAttr.h
)

set(TARGET_SOURCE_FILES
  AddNTAttr.cpp
)

include_directories(
  ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
