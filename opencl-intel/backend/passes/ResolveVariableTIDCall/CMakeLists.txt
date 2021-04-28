set(TARGET_NAME ResolveVariableTIDCall)

set(TARGET_INCLUDE_FILES
  ResolveVariableTIDCall.h
)

set(TARGET_SOURCE_FILES
  ResolveVariableTIDCall.cpp
)

include_directories(
  ${BACKEND_ROOT_DIR}/barrier/include
  ${BACKEND_ROOT_DIR}/utils
  ${CMAKE_BINARY_DIR}/include
)

add_opencl_library(${TARGET_NAME} STATIC
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
