FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

include_directories(include)

link_directories(${TBB_LIB_DIR})

add_ocl_unittest(fpga_test_type SOURCE_FILES ${TARGET_SOURCES}
    LINK_LIBRARIES ${OpenCL_LIBRARY} test_utils intelocl${BUILD_PLATFORM})

add_ocl_unittest_artifacts(
  FILES
    data/internal-ctor-kernel.ioc.obj
    data/internal-ctor-lib.cpp.ioc.obj
)
