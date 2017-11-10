set(TARGET openmp_test_type)

if (NOT WIN32 OR BUILD_FPGA_EMULATOR)
  set( TARGET_LIBS intelocl${BUILD_PLATFORM}${OPENCL_BINARIES_POSTFIX} )
else (NOT WIN32 OR BUILD_FPGA_EMULATOR)
  set( TARGET_LIBS intelocl${BUILD_PLATFORM} )
endif (NOT WIN32 OR BUILD_FPGA_EMULATOR)

# FIXME: for some reason build scripts pull tbb and tbb_malloc to link
# libraries for openmp_test_type, when it is linked directly to
# libintelocl.so. This should not be happening, but let's add link
# tbb directory for now.
link_directories( ${TBB_LIB_DIR} )

add_ocl_unittest(${TARGET}
  SOURCE_FILES
    openmp_test_type.cpp
  LINK_LIBRARIES
    ${TARGET_LIBS}
    test_utils)

add_ocl_unittest_artifacts(
  FILES device)

