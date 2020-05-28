add_llvm_library(gtest_main
  TestMain.cpp

  LINK_LIBS
  gtest

  LINK_COMPONENTS
  Support # Depends on llvm::cl

  # This library is not meant to be in the install tree, only the build tree.
  BUILDTREE_ONLY
  )

# INTEL_CUSTOMIZATION
if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
    add_llvm_library(gtest_main_stdcpp
    TestMain.cpp

    LINK_LIBS
    gtest_stdcpp

    LINK_COMPONENTS
    Support_stdcpp # Depends on llvm::cl

    # This library is not meant to be in the install tree, only the build tree.
    BUILDTREE_ONLY

    STDLIB
    stdc++
    )
endif()
# end INTEL_CUSTOMIZATION

