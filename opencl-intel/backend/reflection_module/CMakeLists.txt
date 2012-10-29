cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

add_subdirectory(unittest)

add_custom_command(
  OUTPUT BuiltinList.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
  BuiltinList.h
  DEPENDS NameManglingTests
  COMMENT "copying MangledNames.h ..."
  VERBATIM
)

add_library(
  ${TARGET_NAME}
  STATIC
  BuiltinKeeper.h
  BuiltinKeeper.cpp
  VersionStrategy.h
  VersionStrategy.cpp
  BuiltinList.h
  BuiltinMap.cpp
  CustomVersionMaping.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/llvm/include
  ${CMAKE_SOURCE_DIR}/backend/vectorizer/Utils
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  LLVMSupport
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )