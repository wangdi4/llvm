set(TARGET_NAME DebugInfoPass)

set(DEBUGINFOPASS_SOURCE_FILES
  DebugInfoPass.cpp
)

include_directories(
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
  ${BACKEND_ROOT_DIR}/utils
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/metadata_api/gen
  ${CMAKE_SOURCE_DIR}/cl_api
)

add_library(${TARGET_NAME}
  ${DEBUGINFOPASS_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
