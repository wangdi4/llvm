set(LLVM_LINK_COMPONENTS BitWriter Core Support TransformUtils)

add_clang_tool(clang-offload-wrapper
  ClangOffloadWrapper.cpp

  DEPENDS
<<<<<<< HEAD
  ${tablegen_deps}
  CUSTOM_WIN_VER
=======
  intrinsics_gen
>>>>>>> 78f60bf4e7f37bf4970bb7bea95ada86e9792d72
  )

set(CLANG_OFFLOAD_WRAPPER_LIB_DEPS
  clangBasic
  )

add_dependencies(clang clang-offload-wrapper)

clang_target_link_libraries(clang-offload-wrapper
  PRIVATE
  ${CLANG_OFFLOAD_WRAPPER_LIB_DEPS}
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(clang-offload-wrapper
    "clang-offload-wrapper.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "Clang offload wrapper.")
endif()
