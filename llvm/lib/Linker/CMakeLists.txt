add_llvm_component_library(LLVMLinker
  IRMover.cpp
  LinkModules.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Linker

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Core
  Object
  Support
  TransformUtils
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
  target_link_libraries(LLVMLinker PRIVATE LLVMIntel_DTrans
                                           LLVMIntel_DTransAnalysis)
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION
