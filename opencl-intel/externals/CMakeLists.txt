add_subdirectory(tbb)
if (NOT OPENCL_INTREE_BUILD)
  add_subdirectory(gtest)
endif()
