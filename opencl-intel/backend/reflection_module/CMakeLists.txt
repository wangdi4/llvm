cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

add_subdirectory(unittest)

set(LINK_CMD
  ln --force ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
  BuiltinList.h
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/BuiltinList.h
  COMMAND ${LINK_CMD}
  DEPENDS NameManglingTests
  COMMENT "linking bi list... ${LINK_CMD}"
  VERBATIM
)

add_library(
  ${TARGET_NAME}
  STATIC
  BuiltinKeeper.cpp
  BuiltinList.h
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/llvm/include
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  LLVMSupport
)
