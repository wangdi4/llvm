use_eh(TRUE)

set(TARGET_NAME name_mangling_test_type)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${OCL_SOURCE_DIR}/backend/name_mangling
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  COMMAND ocl-tblgen -gen-ocl-biheader
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_SOURCE_DIR}/MangledNames.h
  DEPENDS ocl-tblgen
  COMMENT "Genereating demangled headers..."
  VERBATIM
)

add_ocl_unittest(
  ${TARGET_NAME}
  SOURCE_FILES
    MangledNames.h
    MangleTest.cpp
    TestRefCount.cpp
  LINK_LIBRARIES
    name_mangle
    ${LLVM_MODULE_LIBS}
)

