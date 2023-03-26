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

# TODO: The test_utils.a library get undefined symbol errors during linking,
# but the symbols are not actually used.
# W/A is to link cl_sys_utils, cl_hw_utils and intelocl.
if(NOT WIN32)
  target_link_libraries(metadata_api_test_type
    PRIVATE
    cl_sys_utils
    cl_hw_utils
    intelocl${BUILD_PLATFORM}
  )
endif()

add_ocl_unittest_artifacts(FILE metadatatest.ll)
