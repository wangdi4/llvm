cmake_minimum_required(VERSION 2.8.7)

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

set(TARGET_INCLUDE_FILES
  MapList.h
  MetaDataTraits.h
  MetaDataValue.h
  MetaDataObject.h
  MetaDataIterator.h
  MetaDataApiUtils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.h
)

set(TARGET_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/gen/MetaDataApi.cpp
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/gen
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)


if(NOT APPLE)
  add_subdirectory(tests)
endif(NOT APPLE)
