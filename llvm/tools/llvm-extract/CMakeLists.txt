set(LLVM_LINK_COMPONENTS
  BitWriter
  Core
  IPO
  IRReader
  Support
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(DTRANS_ENABLED ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (DTRANS_ENABLED)
  list(APPEND LLVM_LINK_COMPONENTS
    Intel_DTrans
    Intel_DTransAnalysis
    Intel_DTransOpt
  )
endif()
# end INTEL_FEATURE_SW_DTRANS
# end INTEL_CUSTOMIZATION

add_llvm_tool(llvm-extract
  llvm-extract.cpp

  DEPENDS
  intrinsics_gen
  )
