cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

if(NOT APPLE)

if( NOT ANDROID )
  add_subdirectory(unittest)
endif( NOT ANDROID )

if(WIN32)
  set (OCL_TBLGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/${CMAKE_BUILD_TYPE}/ocl-tblgen.exe")
else(WIN32)
  if( LLVM_USE_NATIVE )
    set (OCL_TBLGEN_EXE "${CMAKE_BINARY_DIR}/backend/llvm/bin/ocl-tblgen")
  elseif( CMAKE_CROSSCOMPILING OR ANDROID )
    set (OCL_TBLGEN_EXE "${CMAKE_BINARY_DIR}/native/backend/llvm/bin/ocl-tblgen")
  else( )
    set (OCL_TBLGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/ocl-tblgen")
  endif( LLVM_USE_NATIVE )
endif(WIN32)

add_custom_command(
  OUTPUT BuiltinList.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
  BuiltinList.h
  DEPENDS NameManglingTests
  COMMENT "copying MangledNames.h ..."
  VERBATIM
)

add_custom_command(
  OUTPUT CustomMappings.gen
  COMMAND ${OCL_TBLGEN_EXE} -gen-vectorizer-map
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/KNCni/target-b2.td
  -o ${CMAKE_CURRENT_BINARY_DIR}/CustomMappings.gen
  DEPENDS ${OCL_TBLGEN_EXE}
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v1.2.td
  COMMENT "generating custom mapping ...${OCL_TBLGEN_EXE}"
  VERBATIM
)

endif(NOT APPLE)

if(NOT APPLE)
set(BUILT_IN_LIST_H
  BuiltinList.h
)
set(CUSTOM_MAPPINGS_GEN
  CustomMappings.gen
)
else(NOT APPLE)
set(BUILT_IN_LIST_H
  apple_include/BuiltinList.h
)
endif(NOT APPLE)

set(TARGET_SOURCE_FILES
  BuiltinKeeper.cpp
  VersionStrategy.cpp
  BuiltinMap.cpp
  ${CUSTOM_MAPPINGS_GEN}
)

set(TARGET_INCLUDE_FILES
  BuiltinKeeper.h
  CustomVersionMaping.h
  VersionStrategy.h
  ${BUILT_IN_LIST_H}
  BuiltinMap.h
  utils.h
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
)

if(NOT APPLE)
include_directories(
  ${CMAKE_SOURCE_DIR}/backend/llvm/include
  ${CMAKE_CURRENT_BINARY_DIR}
)
else(NOT APPLE)
include_directories(
  apple_include
)
endif(NOT APPLE)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
