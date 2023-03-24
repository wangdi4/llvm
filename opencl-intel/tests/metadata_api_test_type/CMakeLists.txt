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

# TODO: The test_utils.a library get undefined symbol errors
# using LLD for linker, but the symbols are not actually used, 
# this problem needs more investigation.
if(NOT WIN32)
  target_link_options(metadata_api_test_type PUBLIC "-fuse-ld=bfd")
endif()

add_ocl_unittest_artifacts(FILE metadatatest.ll)
