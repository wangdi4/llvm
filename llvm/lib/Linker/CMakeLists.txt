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
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMLinker PUBLIC LLVMDemangle
                                           LLVMIntel_DTrans
                                           LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION
