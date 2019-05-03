add_llvm_library(LLVMIntel_OptReport
  LoopOptReport.cpp
  LoopOptReportPrintUtils.cpp
  OptReportOptionsPass.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  )

add_dependencies(LLVMIntel_OptReport intrinsics_gen directives_gen)
