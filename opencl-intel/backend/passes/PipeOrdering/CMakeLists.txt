set(TARGET_NAME PipeOrdering)

set(TARGET_INCLUDE_FILES
    PipeOrdering.h
)

set(TARGET_SOURCE_FILES
    PipeOrdering.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/barrier/include
    ${CMAKE_BINARY_DIR}/include/
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
