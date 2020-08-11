add_llvm_component_library(LLVMPasses
  PassBuilder.cpp
  PassPlugin.cpp
  StandardInstrumentations.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Passes

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMPasses PRIVATE LLVMIntel_DTrans
                                           LLVMIntel_DTransOpt
                                           LLVMIntel_DTransAnalysis)
endif(INTEL_INCLUDE_DTRANS)
# end INTEL_CUSTOMIZATION
