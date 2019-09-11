set(TARGET_NAME ResolveSubGroupWICall)

set(TARGET_INCLUDE_FILES
  ResolveSubGroupWICall.h
)

set(TARGET_SOURCE_FILES
  ResolveSubGroupWICall.cpp
)

include_directories(
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
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
