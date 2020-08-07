set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(SYCL_BUILD_PI_CUDA)
 add_subdirectory(cuda)
endif()

add_subdirectory(opencl)
# INTEL_CUSTOMIZATION
if (CMAKE_SYCL_USE_LEVEL0)
if (NOT WIN32)
  add_subdirectory(level_zero)
else()
  add_subdirectory(level_zero_0.9)
endif()
endif()
# end INTEL_CUSTOMIZATION
