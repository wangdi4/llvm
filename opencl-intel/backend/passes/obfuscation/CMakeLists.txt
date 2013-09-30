set(TARGET_NAME Obfuscation)

set(TARGET_SOURCE_FILES Obfuscation.cpp)
set(TARGET_INCLUDE_FILES Obfuscation.h)

include_directories(
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CMAKE_SOURCE_DIR}/backend/passes
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/metadata_api
)

add_library(${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME}
  metadataapi
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
