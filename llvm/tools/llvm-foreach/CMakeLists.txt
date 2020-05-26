set(LLVM_LINK_COMPONENTS
  Support
  )

add_llvm_tool(llvm-foreach
  llvm-foreach.cpp
  CUSTOM_WIN_VER
)

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(llvm-foreach
    "llvm-foreach.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "Multiple command line processing tool.")
endif()
