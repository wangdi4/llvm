add_llvm_library(LLVMIntel_VPOTransforms
  VPOTransform.cpp

  Utils/CFGRestructuring.cpp
  Utils/IntrinsicUtils.cpp
  Utils/OpenMPDirectivesAndClauses.cpp
#  Utils/GeneralUtils.cpp // moved to Transforms/Utils
  Utils/MultiVersioning.cpp

  Vecopt/VPODriver.cpp
  Vecopt/VPOAvrLLVMCodeGen.cpp
  Vecopt/VPOAvrHIRCodeGen.cpp

  Paropt/VPOParopt.cpp
  Paropt/VPOParoptTransform.cpp
  Paropt/VPOParoptUtils.cpp
)

add_dependencies(LLVMIntel_VPOTransforms intrinsics_gen)
