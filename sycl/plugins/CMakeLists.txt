add_subdirectory(opencl)
# INTEL_CUSTOMIZATION
if (CMAKE_SYCL_USE_LEVEL0)
  add_subdirectory( Intel_level0 )
endif()
# end INTEL_CUSTOMIZATION
