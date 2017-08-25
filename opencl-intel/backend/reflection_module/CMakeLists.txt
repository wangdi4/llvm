cmake_minimum_required(VERSION 2.8.7)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

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
  set(OCL_TBLGEN_DEP ConfigureNativeTargets)
  set(OCL_TBLGEN ${CMAKE_BINARY_DIR}/native/bin/ocl-tblgen)
else()
  set(OCL_TBLGEN_DEP ocl-tblgen)
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
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0.td
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0WG.td
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/WorkGroupSnippets.td
  COMMENT "generating custom mapping ..."
  VERBATIM
)

set(BUILT_IN_LIST_H
  BuiltinList.h
)
set(CUSTOM_MAPPINGS_GEN
  CustomMappings.gen
)

set(TARGET_SOURCE_FILES
  Mangler.cpp
  BuiltinKeeper.cpp
  VersionStrategy.cpp
  BuiltinMap.cpp
  ${CUSTOM_MAPPINGS_GEN}
)

set(TARGET_INCLUDE_FILES
  Mangler.h
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
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_SOURCE_DIR}/backend/utils
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
