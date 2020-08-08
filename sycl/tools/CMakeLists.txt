add_subdirectory(sycl-ls)

# TODO: move each tool in its own sub-directory
add_executable(get_device_count_by_type get_device_count_by_type.cpp)
# INTEL_CUSTOMIZATON
if(MSVC)
  add_dependencies(get_device_count_by_type ocl-headers ocl-icd l0-loader)
else()
  add_dependencies(get_device_count_by_type ocl-headers ocl-icd level-zero-loader)
endif()
# end INTEL_CUSTOMIZATON


if(MSVC)
  set(LEVEL_ZERO_LIBRARY
    "${LLVM_LIBRARY_OUTPUT_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}ze_loader${CMAKE_STATIC_LIBRARY_SUFFIX}")
# INTEL_CUSTOMIZATON
   set(LEVEL_ZERO_HEADERS L0Loader::Headers)
# end INTEL_CUSTOMIZATON
else()
   set(LEVEL_ZERO_LIBRARY
     "${LLVM_LIBRARY_OUTPUT_INTDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ze_loader${CMAKE_SHARED_LIBRARY_SUFFIX}")
# INTEL_CUSTOMIZATON
  set(LEVEL_ZERO_HEADERS LevelZeroLoader::Headers)
# end INTEL_CUSTOMIZATON
endif()

target_link_libraries(get_device_count_by_type
  PRIVATE
    OpenCL::Headers
    # INTEL_CUSTOMIZATON
     ${LEVEL_ZERO_HEADERS}
    # end INTEL_CUSTOMIZATON
    ${OpenCL_LIBRARIES}
    ${LEVEL_ZERO_LIBRARY}
    $<$<BOOL:${SYCL_BUILD_PI_CUDA}>:cudadrv>
)
target_compile_definitions(get_device_count_by_type
  PRIVATE
    $<$<BOOL:${SYCL_BUILD_PI_CUDA}>:USE_PI_CUDA>
)
