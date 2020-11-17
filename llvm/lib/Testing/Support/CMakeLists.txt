add_llvm_library(LLVMTestingSupport
  Annotations.cpp
  Error.cpp
  SupportHelpers.cpp

  BUILDTREE_ONLY

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

  LINK_COMPONENTS
  Support
  )

# INTEL_CUSTOMIZATION
if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
  add_llvm_library(LLVMTestingSupport_stdcpp
    Annotations.cpp
    Error.cpp
    SupportHelpers.cpp

    BUILDTREE_ONLY

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

    LINK_COMPONENTS
    Support_stdcpp

    STDLIB
    stdc++
  )
  target_link_libraries(LLVMTestingSupport_stdcpp PRIVATE gtest_stdcpp)
endif()
if (WIN32)
  if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(_STL_LIB "/MDd")
  else()
    set(_STL_LIB "/MD")
  endif()
  add_llvm_library(LLVMTestingSupport_dyn
    Annotations.cpp
    Error.cpp
    SupportHelpers.cpp

    BUILDTREE_ONLY

    ADDITIONAL_HEADER_DIRS
    ${LLVM_MAIN_INCLUDE_DIR}/llvm/Testing/Support

    LINK_COMPONENTS
    Support_dyn

    STDLIB
    ${_STL_LIB}
  )
  target_link_libraries(LLVMTestingSupport_dyn PRIVATE gtest_dyn)
endif()
# end INTEL_CUSTOMIZATION
target_link_libraries(LLVMTestingSupport PRIVATE gtest)
