cmake_minimum_required(VERSION 2.8)

set(TARGET_NAME reflection_module)

project (${TARGET_NAME})

#enables compound initialization literals
remove_definitions("-pedantic")

if( NOT ANDROID )
    add_subdirectory(unittest)
endif()

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

if (NOT WIN32)
  if( NOT ANDROID)
  target_link_libraries(${TARGET_NAME} pthread)
  endif()
endif (NOT WIN32)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
