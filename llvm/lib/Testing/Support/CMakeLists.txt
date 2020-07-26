add_definitions(-DGTEST_LANG_CXX11=1)
add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

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

<<<<<<< HEAD
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
# end INTEL_CUSTOMIZATION
if (CXX_SUPPORTS_SUGGEST_OVERRIDE_FLAG)
  add_compile_options("-Wno-suggest-override")
endif()

=======
>>>>>>> 97a0f80c469c338e9173355e9f8a0a977adfe266
include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include)
include_directories(${LLVM_MAIN_SRC_DIR}/utils/unittest/googlemock/include)
target_link_libraries(LLVMTestingSupport PRIVATE gtest)
