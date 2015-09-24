add_llvm_library(LLVMipa
#ifdef INTEL_CUSTOMIZATION
  Andersens.cpp
#endif     // INTEL_CUSTOMIZATION
  CallGraph.cpp
  CallGraphSCCPass.cpp
  CallPrinter.cpp
  GlobalsModRef.cpp
  IPA.cpp
  InlineCost.cpp
  )

add_dependencies(LLVMipa intrinsics_gen)
