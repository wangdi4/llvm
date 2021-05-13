FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

include_directories(include ${OCL_SOURCE_DIR}/tests/framework_test_type)

link_directories(${TBB_LIB_DIR})

add_ocl_unittest(fpga_test_type
  SOURCE_FILES
    ${TARGET_SOURCES}
    ${OCL_SOURCE_DIR}/tests/framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES
    OpenCL-ICD
    test_utils
    intelocl${BUILD_PLATFORM}
)

if (WIN32)
  set_target_properties(fpga_test_type PROPERTIES
    LINK_FLAGS_RELEASE "${LINK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
    LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
  )
endif()

add_ocl_unittest_artifacts(
  FILES
    data/internal-ctor-kernel.ioc.obj
    data/internal-ctor-lib.cpp.ioc.obj
    data/atomics64.spv
)
