add_custom_target(SYCLUnitTests)
set_target_properties(SYCLUnitTests PROPERTIES FOLDER "SYCL tests")

foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
# INTEL_CUSTOMIZATION
string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
# end INTEL_CUSTOMIZATION
endforeach()

# add_sycl_unittest(test_dirname SHARED|OBJECT file1.cpp, file2.cpp ...)
#
# Will compile the list of files together and link against SYCL.
# Produces a binary names `basename(test_dirname)`.
function(add_sycl_unittest test_dirname link_variant)
  # Enable exception handling for these unit tests
  set(LLVM_REQUIRES_EH 1)

# INTEL_CUSTOMIZATION
  if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(sycl_obj_target "syclmtd_object")
    set(sycl_so_target "syclmtd")
  elseif (MSVC)
    set(sycl_obj_target "syclmt_object")
    set(sycl_so_target "syclmt")
#end INTEL_CUSTOMIZATION
  else()
    set(sycl_obj_target "sycl_object")
    set(sycl_so_target "sycl")
  endif()

  if ("${link_variant}" MATCHES "SHARED")
    set(SYCL_LINK_LIBS ${sycl_so_target})
    add_unittest(SYCLUnitTests ${test_dirname} ${ARGN})
  else() 
    add_unittest(SYCLUnitTests ${test_dirname} 
                $<TARGET_OBJECTS:${sycl_obj_target}> CUSTOM_WIN_VER ${ARGN})
    target_compile_definitions(${test_dirname} PRIVATE __SYCL_BUILD_SYCL_DLL)

    get_target_property(SYCL_LINK_LIBS ${sycl_so_target} LINK_LIBRARIES)
  endif()

  # INTEL_CUSTOMIZATION
  if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
    set(TESTING_SUPPORT_LIB LLVMTestingSupport_stdcpp)
  else()
    set(TESTING_SUPPORT_LIB LLVMTestingSupport)
  endif()

  target_link_libraries(${test_dirname}
    PRIVATE
      ${TESTING_SUPPORT_LIB}
      OpenCL-Headers
      ${SYCL_LINK_LIBS}
    )
  # end INTEL_CUSTOMIZATION
  target_include_directories(${test_dirname} PRIVATE SYSTEM
      ${sycl_inc_dir}
      ${SYCL_SOURCE_DIR}/source/
      ${SYCL_SOURCE_DIR}/unittests/
    )
  # LLVM gtest uses LLVM utilities that require C++-14
  # CXX_STANDARD_REQUIRED makes CXX_STANDARD a hard requirement.
  set_target_properties(${test_dirname}
    PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
    )
endfunction()

add_subdirectory(misc)
add_subdirectory(pi)
add_subdirectory(program)
add_subdirectory(queue)
add_subdirectory(scheduler)
add_subdirectory(thread_safety)
