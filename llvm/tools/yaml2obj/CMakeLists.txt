set(LLVM_LINK_COMPONENTS
  ObjectYAML
  Support
  )
# INTEL_CUSTOMIZATION
add_llvm_tool(yaml2obj
# end INTEL_CUSTOMIZATION
  yaml2obj.cpp
  )
