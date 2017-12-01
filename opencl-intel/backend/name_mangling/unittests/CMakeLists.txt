use_eh(TRUE)

set(TARGET_NAME NameManglingTests)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../externals/gtest/include
  ${BACKEND_ROOT_DIR}/name_mangling
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  COMMAND ocl-tblgen -gen-ocl-biheader
  -I ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${CMAKE_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  DEPENDS ocl-tblgen
  COMMENT "Genereating demangled headers..."
  VERBATIM
)

add_executable(
  ${TARGET_NAME}
  MangledNames.h
  MangleTest.cpp
  TestRefCount.cpp
)

target_link_libraries(
  ${TARGET_NAME}
  name_mangle
  googletest
  ${LLVM_MODULE_LIBS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_TESTS_FOLDER_NAME} )

install(FILES NameManglingTests.pm DESTINATION tests/name_mangling)
install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/name_mangling)
