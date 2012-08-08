cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

add_subdirectory(unittest)

if (WIN32)
  set(LINK_CMD
    ln --force
    ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
    BuiltinList.h
  )
else(WIN32)
  set(LINK_CMD
    ln -s --force ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
    BuiltinList.h
  )
endif(WIN32)

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
  utils.h
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
