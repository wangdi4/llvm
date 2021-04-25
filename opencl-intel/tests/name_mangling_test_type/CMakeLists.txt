use_eh(TRUE)

set(TARGET_NAME name_mangling_test_type)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${LLVM_MAIN_SRC_DIR}/lib/Transforms/Intel_DPCPPKernelTransforms/Utils
  ${OCL_SOURCE_DIR}/backend/name_mangling
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MangledNames.inc
  COMMAND ocl-tblgen -gen-ocl-biheader
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/GENERIC/GENERIC_CPU.td
  -o ${CMAKE_CURRENT_BINARY_DIR}/MangledNames.inc
  DEPENDS ocl-tblgen
  COMMENT "Generating demangled headers..."
  VERBATIM
)

add_ocl_unittest(
  ${TARGET_NAME}
  SOURCE_FILES
    MangledNames.inc
    MangleTest.cpp
  LINK_LIBRARIES
    ${LLVM_MODULE_LIBS}
)

