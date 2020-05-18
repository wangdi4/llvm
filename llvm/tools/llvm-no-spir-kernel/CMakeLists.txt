set(LLVM_LINK_COMPONENTS
  Core
  Demangle
  IRReader
  Support
  )

add_llvm_tool(llvm-no-spir-kernel
  llvm-no-spir-kernel.cpp

  CUSTOM_WIN_VER
  DEPENDS
  intrinsics_gen
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(llvm-no-spir-kernel
    "llvm-no-spir-kernel.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "LLVM SPIR kernel checking tool.")
endif()
