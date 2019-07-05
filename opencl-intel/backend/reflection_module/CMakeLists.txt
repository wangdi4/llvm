set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

if( BACKEND_BUILD_VERIFICATION_LIB )
    add_subdirectory(unittest)
endif( BACKEND_BUILD_VERIFICATION_LIB )

add_custom_command(
  OUTPUT BuiltinList.inc
  COMMAND ${CMAKE_COMMAND} -E copy ${OCL_BINARY_DIR}/tests/name_mangling_test_type/MangledNames.inc
  BuiltinList.inc
  DEPENDS name_mangling_test_type
  COMMENT "copying MangledNames.inc ..."
  VERBATIM
)

add_custom_command(
  OUTPUT CustomMappings.gen
  COMMAND ocl-tblgen -gen-vectorizer-map
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_BINARY_DIR}/CustomMappings.gen
  DEPENDS ocl-tblgen
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/soa_relational.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v1.2.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0.td
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/opencl-v2.0WG.td
  COMMENT "generating custom mapping ..."
  VERBATIM
)

set(BUILT_IN_LIST_INC
  BuiltinList.inc
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
  ${BUILT_IN_LIST_INC}
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
  ${OCL_SOURCE_DIR}/backend/name_mangling
  ${OCL_SOURCE_DIR}/backend/utils
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
