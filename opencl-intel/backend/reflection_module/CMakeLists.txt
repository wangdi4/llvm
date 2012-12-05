cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

if(NOT APPLE)
	add_subdirectory(unittest)
endif(NOT APPLE)


#set(GenCommand ~/usr/local/bin/ocl-tblgen -gen-ocl-biheader 
#  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
#  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/target-knc.td
#  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h)
#
#add_custom_command(
#  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
#  COMMAND ${GenCommand}
#  DEPENDS ocl-tblgen
#  COMMENT "genereating demangled headers... ${GenCommand}"
#  VERBATIM
#)
#
#
#add_custom_command(
#  OUTPUT BuiltinList.h
#  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
#  BuiltinList.h
#  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
#  COMMENT "copying MangledNames.h ..."
#  VERBATIM
#)
#
#add_custom_command(
#  OUTPUT CustomMappings.gen
#  COMMAND ~/usr/local/bin/ocl-tblgen -gen-vectorizer-map
#  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
#  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/target-knc.td
#  -o ${CMAKE_CURRENT_BINARY_DIR}/CustomMappings.gen
#  DEPENDS
#  ocl-tblgen
#  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
#  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/opencl-v1.2.td
#  COMMENT "generating custom mapping ..."
#  VERBATIM
#)

add_library(
  ${TARGET_NAME}
  STATIC
  BuiltinKeeper.h
  BuiltinKeeper.cpp
  VersionStrategy.h
  VersionStrategy.cpp
  BuiltinList.h
  BuiltinMap.cpp
  CustomMappings.gen
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/vectorizer/Utils
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  LLVMSupport
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
