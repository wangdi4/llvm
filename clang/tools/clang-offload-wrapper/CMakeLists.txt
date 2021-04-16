if(INTEL_COLLAB)
set(LLVM_LINK_COMPONENTS BitWriter Core Object Support TransformUtils)
else(INTEL_COLLAB)
set(LLVM_LINK_COMPONENTS BitWriter Core Support TransformUtils)
endif(INTEL_COLLAB)
if(INTEL_CUSTOMIZATION)
list(APPEND LLVM_LINK_COMPONENTS ObjectYAML)
endif(INTEL_CUSTOMIZATION)
add_clang_tool(clang-offload-wrapper
  ClangOffloadWrapper.cpp

  DEPENDS
  intrinsics_gen
  CUSTOM_WIN_VER
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
