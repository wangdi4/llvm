use_rtti( FALSE )
set( TARGET_NAME LoopUtils)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/reflection_module
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/metadata_api/gen
  ${CMAKE_SOURCE_DIR}/cl_api
)

set(TARGET_SOURCE_FILES
  LoopUtils.cpp
  LoopUtils.h
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} metadataapi)

# For a tidy Visual Studio solution layout
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
