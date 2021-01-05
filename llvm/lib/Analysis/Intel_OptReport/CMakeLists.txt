add_llvm_component_library(LLVMIntel_OptReport
  Diag.cpp
  LoopOptReport.cpp
  LoopOptReportPrintUtils.cpp
  OptReportOptionsPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis

  LINK_COMPONENTS
  Analysis
  Core
  Support
  ProfileData
  Object
  )

add_dependencies(LLVMIntel_OptReport intrinsics_gen directives_gen)
