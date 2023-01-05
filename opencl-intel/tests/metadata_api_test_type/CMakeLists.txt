use_rtti(FALSE)

include_directories(${LLVM_INCLUDE_DIRS})

add_ocl_unittest(
  metadata_api_test_type
  SOURCE_FILES
  MetadataAPITest.cpp
  MetadataAPITestFixture.h
  LINK_LIBRARIES
  LLVMCore
  LLVMIRReader
  LLVMSupport
  LLVMTransformUtils
  test_utils)

add_ocl_unittest_artifacts(FILE metadatatest.ll)
