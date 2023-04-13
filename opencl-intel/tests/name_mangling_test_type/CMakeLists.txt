use_eh(TRUE)

set(TARGET_NAME name_mangling_test_type)

include_directories(
  ${LLVM_INCLUDE_DIRS} ${OCL_SOURCE_DIR}/backend/name_mangling
  ${OCL_BINARY_DIR}/include)

add_ocl_unittest(${TARGET_NAME} SOURCE_FILES MangleTest.cpp LINK_LIBRARIES
                 LLVMSupport LLVMSYCLTransforms)

add_dependencies(${TARGET_NAME} reflection_module)
