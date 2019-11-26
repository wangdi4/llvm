add_custom_target(PiUnitTests)
set_target_properties(PiUnitTests PROPERTIES FOLDER "Tests")

function(add_llvm_unittest test_dirname)
  add_unittest(PiUnitTests ${test_dirname} ${ARGN})
endfunction()

<<<<<<< HEAD
function(add_llvm_unittest_with_input_files test_dirname)
  add_unittest_with_input_files(PiUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(pi)
=======
  if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
    set(sycl_lib "sycld")
  else()
    set(sycl_lib "sycl")
  endif()

  add_unittest(SYCLUnitTests ${test_dirname} ${ARGN})
  target_link_libraries(${test_dirname}
    PRIVATE
      ${sycl_lib}
      LLVMTestingSupport
      OpenCL-Headers
    )
  target_include_directories(${test_dirname} PRIVATE SYSTEM ${sycl_inc_dir})
  # LLVM gtest uses LLVM utilities that require C++-14
  # CXX_STANDARD_REQUIRED makes CXX_STANDARD a hard requirement.
  set_target_properties(${test_dirname}
    PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
    )
endfunction()

add_subdirectory(pi)
add_subdirectory(misc)
>>>>>>> 2abb131d5e397e1b7f5538994388ffc23920b55e
