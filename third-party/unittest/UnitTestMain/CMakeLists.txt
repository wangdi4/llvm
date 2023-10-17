set(BUILDTREE_ONLY BUILDTREE_ONLY)
if (LLVM_INSTALL_GTEST)
  set(BUILDTREE_ONLY "")
endif ()

add_llvm_library(llvm_gtest_main
  TestMain.cpp

  LINK_LIBS
  llvm_gtest

  LINK_COMPONENTS
  Support # Depends on llvm::cl

  ${BUILDTREE_ONLY}
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
    add_llvm_library(llvm_gtest_main_dyn
    TestMain.cpp

    LINK_LIBS
    llvm_gtest_dyn

    LINK_COMPONENTS
    Support_dyn # Depends on llvm::cl

    # This library is not meant to be in the install tree, only the build tree.
    BUILDTREE_ONLY
    )
    set_property(TARGET llvm_gtest_main_dyn
                 PROPERTY MSVC_RUNTIME_LIBRARY  "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
# end INTEL_CUSTOMIZATION

