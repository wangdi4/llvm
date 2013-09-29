set(TARGET_NAME Obfuscation)

include_directories(
  ${CMAKE_SOURCE_DIR}/cl_api
  ${CMAKE_SOURCE_DIR}/backend/passes
  ${CMAKE_SOURCE_DIR}/backend/utils
  ${CMAKE_SOURCE_DIR}/backend/metadata_api
)

add_library(${TARGET_NAME}
  STATIC
  Obfuscation.cpp
  Obfuscation.h
)

add_dependencies(${TARGET_NAME}
  metadataapi
  OclBackendUtils
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
