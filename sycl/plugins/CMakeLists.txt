set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(SYCL_BUILD_PI_CUDA)
 add_subdirectory(cuda)
endif()

add_subdirectory(opencl)
<<<<<<< HEAD
# INTEL_CUSTOMIZATION
if (CMAKE_SYCL_USE_LEVEL0)
  add_subdirectory( level_zero )
endif()
# end INTEL_CUSTOMIZATION

=======
add_subdirectory(level_zero)
>>>>>>> d32da99fed6595c11c4be40f8db2312e11b07cdf
