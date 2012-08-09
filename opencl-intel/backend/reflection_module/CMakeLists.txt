cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

add_subdirectory(unittest)

#we assume the existance of Cygwin, or some other linux simulation layer in
#WINDOWS
add_custom_command(
  OUTPUT BuiltinList.h
  COMMAND cp ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
  BuiltinList.h
  DEPENDS NameManglingTests
  COMMENT "copying MangledNames.h ..."
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
