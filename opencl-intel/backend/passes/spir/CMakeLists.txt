set(TARGET_NAME Materialize)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/passes
  ${CMAKE_SOURCE_DIR}/backend/metadata_api
)

add_library(${TARGET_NAME}
  STATIC
  Materialize.cpp
  Materialize.h
)
  
add_dependencies(${TARGET_NAME}
  metadataapi
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
