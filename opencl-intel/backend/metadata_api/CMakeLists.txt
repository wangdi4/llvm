cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME metadataapi)
set(MAKO_GENERATOR_CMD python ${CMAKE_CURRENT_SOURCE_DIR}/generator.py) 

set ( SCHEMA_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApiIntel.schema)

add_custom_command(
   OUTPUT  ${CMAKE_BINARY_DIR}/include/MetaDataApi.h
   COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/include/
   COMMAND ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.h.mak ${SCHEMA_FILE} -o ${CMAKE_BINARY_DIR}/include/MetaDataApi.h
   DEPENDS MetaDataApi.h.mak
   DEPENDS MetaDataApi.def.mak
   DEPENDS ${SCHEMA_FILE}
   COMMENT "running ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.h.mak ${SCHEMA_FILE} -o ${CMAKE_BINARY_DIR}/include/MetaDataApi.h"
)

add_custom_command(
   OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/MetaDataApi.cpp
   COMMAND ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.cpp.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/MetaDataApi.cpp
   DEPENDS MetaDataApi.cpp.mak
   DEPENDS MetaDataApi.def.mak
   DEPENDS ${SCHEMA_FILE}
   COMMENT "running ${MAKO_GENERATOR_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/MetaDataApi.cpp.mak ${SCHEMA_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/MetaDataApi.cpp"
)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/MetaDataApi.cpp PROPERTIES GENERATED 1)
set_source_files_properties(${CMAKE_BINARY_DIR}/include/MetaDataApi.h PROPERTIES GENERATED 1)

set(TARGET_INCLUDE_FILES
  MapList.h
  MetaDataTraits.h
  MetaDataValue.h
  MetaDataObject.h
  MetaDataIterator.h
  MetaDataApiUtils.h
  ${CMAKE_BINARY_DIR}/include/MetaDataApi.h
)

set(TARGET_SOURCE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/MetaDataApi.cpp
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )

add_subdirectory(tests)
