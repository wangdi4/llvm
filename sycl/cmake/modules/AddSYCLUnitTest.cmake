# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
# add_sycl_unittest(test_dirname SHARED|OBJECT file1.cpp, file2.cpp ...)
#
# Will compile the list of files together and link against SYCL.
# Produces a binary names `basename(test_dirname)`.
macro(add_sycl_unittest test_dirname link_variant)
  # Enable exception handling for these unit tests
  set(LLVM_REQUIRES_EH ON)
  set(LLVM_REQUIRES_RTTI ON)

  string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
  if (MSVC AND build_type_lower MATCHES "debug")
    set(sycl_obj_target "sycld_object")
    set(sycl_so_target "sycld")
  else()
    set(sycl_obj_target "sycl_object")
    set(sycl_so_target "sycl")
  endif()

  if ("${link_variant}" MATCHES "SHARED")
    set(SYCL_LINK_LIBS ${sycl_so_target})
    if(INTEL_CUSTOMIZATION)
        add_unittest(SYCLUnitTests ${test_dirname} CUSTOM_WIN_VER ${ARGN})
    else(INTEL_CUSTOMIZATION)
        add_unittest(SYCLUnitTests ${test_dirname} ${ARGN})
    endif(INTEL_CUSTOMIZATION)
  else()
    if(INTEL_CUSTOMIZATION)
        add_unittest(SYCLUnitTests ${test_dirname}
                    $<TARGET_OBJECTS:${sycl_obj_target}> CUSTOM_WIN_VER ${ARGN})
    else(INTEL_CUSTOMIZATION)
        add_unittest(SYCLUnitTests ${test_dirname}
                    $<TARGET_OBJECTS:${sycl_obj_target}> ${ARGN})
    endif(INTEL_CUSTOMIZATION)
    target_compile_definitions(${test_dirname} PRIVATE __SYCL_BUILD_SYCL_DLL)

    get_target_property(SYCL_LINK_LIBS ${sycl_so_target} LINK_LIBRARIES)
  endif()

# INTEL_CUSTOMIZATION
  if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
    set(TESTING_SUPPORT_LIB LLVMTestingSupport_stdcpp)
  elseif(WIN32)
    set(TESTING_SUPPORT_LIB LLVMTestingSupport_dyn)
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
      target_compile_options(${test_dirname} PRIVATE "/MDd")
    else()
      target_compile_options(${test_dirname} PRIVATE "/MD")
    endif()
  else()
    set(TESTING_SUPPORT_LIB LLVMTestingSupport)
  endif()

  if (SYCL_ENABLE_COVERAGE)
    target_compile_options(${test_dirname} PUBLIC
      -fprofile-instr-generate -fcoverage-mapping
    )
    target_link_options(${test_dirname} PUBLIC
      -fprofile-instr-generate -fcoverage-mapping
    )
  endif()

  add_custom_target(check-sycl-${test_dirname}
    ${CMAKE_COMMAND} -E env
    LLVM_PROFILE_FILE="${SYCL_COVERAGE_PATH}/${test_dirname}.profraw"
    env SYCL_CONFIG_FILE_NAME=null.cfg
    env SYCL_DEVICELIB_NO_FALLBACK=1
    env SYCL_CACHE_DIR="${CMAKE_BINARY_DIR}/sycl_cache"
    ${CMAKE_CURRENT_BINARY_DIR}/${test_dirname}
    DEPENDS
    ${test_dirname}
  )

  add_dependencies(check-sycl-unittests check-sycl-${test_dirname})

  target_link_libraries(${test_dirname}
    PRIVATE
      ${TESTING_SUPPORT_LIB}
      OpenCL-Headers
      ${SYCL_LINK_LIBS}
    )
# end INTEL_CUSTOMIZATION

  target_include_directories(${test_dirname}
    PRIVATE SYSTEM
      ${sycl_inc_dir}
      ${SYCL_SOURCE_DIR}/source/
      ${SYCL_SOURCE_DIR}/unittests/
    )
  if (UNIX)
    # These warnings are coming from Google Test code.
    target_compile_options(${test_dirname}
      PRIVATE
        -Wno-unused-parameter
        -Wno-inconsistent-missing-override
    )
  endif()
endmacro()
