set(LLVM_LINK_COMPONENTS
  Support
  )

add_llvm_tool(file-table-tform
  file-table-tform.cpp
  CUSTOM_WIN_VER
  )

if(INTEL_CUSTOMIZATION)
  dpcpptarget_add_resource_file(file-table-tform
    "file-table-tform.exe"
    "Intel(R) oneAPI DPC++ Compiler"
    "File table transformation tool.")
endif()
