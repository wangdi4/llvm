cmake_minimum_required(VERSION 2.8.7)
use_eh(TRUE)

set(TARGET_NAME NameManglingTests)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${BACKEND_ROOT_DIR}/name_mangling
)

if(CMAKE_CROSSCOMPILING)
  set(OCL_TBLGEN_DEP ConfigureNativeTargets)
  set(OCL_TBLGEN ${CX_NATIVE_TG_DIR}/bin/ocl-tblgen)
else()
  set(OCL_TBLGEN_DEP ocl-tblgen)
  set(OCL_TBLGEN ocl-tblgen)
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  COMMAND ${OCL_TBLGEN} -gen-ocl-biheader 
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  DEPENDS ${OCL_TBLGEN_DEP}
  COMMENT "Genereating demangled headers..."
  VERBATIM
)

add_executable(
  ${TARGET_NAME}
  MangledNames.h
  MangleTest.cpp
  TestRefCount.cpp
)

if( ANDROID ) 
  target_link_libraries( ${TARGET_NAME} gnustl_shared )
  set_property(TARGET ${TARGET_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -pie")
endif( ANDROID )

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  googletest
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES NameManglingTests.pm DESTINATION tests/name_mangling)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/name_mangling)
