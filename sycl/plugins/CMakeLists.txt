set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-covered-switch-default")
endif()

if(SYCL_BUILD_PI_CUDA)
  add_subdirectory(cuda)
endif()

add_subdirectory(opencl)
# INTEL_CUSTOMIZATION
if (CMAKE_SYCL_USE_LEVEL0)
  add_subdirectory(level_zero)
endif()
# end INTEL_CUSTOMIZATION
