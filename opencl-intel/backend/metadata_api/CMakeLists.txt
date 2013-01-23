cmake_minimum_required(VERSION 2.8.4)

set(TARGET_NAME metadataapi)
set(MAKO_GENERATOR_CMD python ${CMAKE_CURRENT_SOURCE_DIR}/generator.py) 

if (APPLE)
    set ( SCHEMA_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApiApple.schema)
else (APPLE)
    set ( SCHEMA_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApiIntel.schema)
endif (APPLE)

add_custom_command(
   OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h
   COMMAND ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.h.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h
   DEPENDS MetaDataApi.h.mak
   DEPENDS MetaDataApi.def.mak
   DEPENDS ${SCHEMA_FILE}
   COMMENT "running ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.h.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h"
)

add_custom_command(
   OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp
   COMMAND ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.cpp.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp
   DEPENDS MetaDataApi.cpp.mak
   DEPENDS MetaDataApi.def.mak
   DEPENDS ${SCHEMA_FILE}
   COMMENT "running ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.cpp.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp"
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h PROPERTIES GENERATED 1)

add_library(
  ${TARGET_NAME} STATIC
  MetaDataTraits.h
  MetaDataValue.h
  MetaDataObject.h
  MetaDataIterator.h
  MetaDataApiUtils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/gen
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

add_subdirectory(tests)
