set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(get_device_count_by_type get_device_count_by_type.cpp)
add_dependencies(get_device_count_by_type ocl-headers ocl-icd)
target_link_libraries(get_device_count_by_type
    PRIVATE OpenCL::Headers
    PRIVATE ${OpenCL_LIBRARIES}
)

add_executable(sycl-check sycl-check.cpp)
add_dependencies(sycl-check sycl)
target_include_directories(sycl-check PRIVATE "${sycl_inc_dir}")
target_link_libraries(sycl-check
    PRIVATE sycl
    PRIVATE OpenCL::Headers
    PRIVATE ${OpenCL_LIBRARIES})

#Minimum supported version of Intel's OCL GPU and CPU devices
target_compile_definitions(sycl-check
     PRIVATE  MIN_INTEL_OCL_GPU_VERSION=\"18.47.11882\"
     PRIVATE  MIN_INTEL_OCL_CPU_VERSION=\"18.1.0.0901\",\"7.6.0.1202\"
)
<<<<<<< HEAD

# INTEL_CUSTOMIZATION CORC-4868
#
# Do not treat warnings as error unless it fixed upstream
#
target_compile_options (sycl-check PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-error>
    $<$<CXX_COMPILER_ID:MSVC>:/WX->
)
#end INTEL_CUSTOMIZATION
=======
>>>>>>> 67c6ae5594ae14f9b00f491b7b25c244bbace88a
