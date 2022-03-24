add_llvm_library(llvm_gtest_main
  TestMain.cpp

  LINK_LIBS
  llvm_gtest

  LINK_COMPONENTS
  Support # Depends on llvm::cl

  # This library is not meant to be in the install tree, only the build tree.
  BUILDTREE_ONLY
  )

# INTEL_CUSTOMIZATION
if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
    add_llvm_library(llvm_gtest_main_stdcpp
    TestMain.cpp

    LINK_LIBS
    llvm_gtest_stdcpp

    LINK_COMPONENTS
    Support_stdcpp # Depends on llvm::cl

    # This library is not meant to be in the install tree, only the build tree.
    BUILDTREE_ONLY

    STDLIB
    stdc++
    )
endif()
if (WIN32)
    if (${CMAKE_BUILD_TYPE} MATCHES Debug)
      set(_STL_LIB "/MDd")
    else()
      set(_STL_LIB "/MD")
    endif()
    add_llvm_library(llvm_gtest_main_dyn
    TestMain.cpp

    LINK_LIBS
    llvm_gtest_dyn

    LINK_COMPONENTS
    Support_dyn # Depends on llvm::cl

    # This library is not meant to be in the install tree, only the build tree.
    BUILDTREE_ONLY

    STDLIB
    ${_STL_LIB}
    )
endif()
# end INTEL_CUSTOMIZATION

