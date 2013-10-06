cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

if(NOT APPLE)
if( BACKEND_BUILD_VERIFICATION_LIB AND (NOT ANDROID) )
    add_subdirectory(unittest)
endif( BACKEND_BUILD_VERIFICATION_LIB AND (NOT ANDROID) )

add_custom_command(
  OUTPUT BuiltinList.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/backend/name_mangling/unittests/MangledNames.h
  BuiltinList.h
  DEPENDS NameManglingTests
  COMMENT "copying MangledNames.h ..."
  VERBATIM
)

 if(CMAKE_CROSSCOMPILING)
    set(OCL_TBLGEN_DEP ocl-tblgen-native)
    set(OCL_TBLGEN ${CMAKE_BINARY_DIR}/native/bin/ocl-tblgen)
 else()
    set(OCL_TBLGEN_DEP ocl-tblgen)
    set(OCL_TBLGEN ocl-tblgen)
 endif()

if(CMAKE_CROSSCOMPILING)
    set(OCL_TBLGEN ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen)
else()
    set(OCL_TBLGEN ocl-tblgen)
endif()

add_custom_command(
  OUTPUT CustomMappings.gen
  COMMAND ${OCL_TBLGEN} -gen-vectorizer-map
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_BINARY_DIR}/CustomMappings.gen
  DEPENDS ${OCL_TBLGEN_DEP}
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v1.2.td
  COMMENT "generating custom mapping ..."
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
  ${CMAKE_CURRENT_BINARY_DIR}
)
else(NOT APPLE)
include_directories(
  apple_include
)
endif(NOT APPLE)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
