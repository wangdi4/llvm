# INTEL_COLLAB
add_llvm_library(LLVMIntel_VPOTransforms
  VPOTransform.cpp

  Utils/CFGRestructuring.cpp
  Utils/MultiVersioning.cpp
  Utils/ParSecTrans.cpp
# Utils/IntrinsicUtils.cpp // moved to Transforms/Utils/CMakeLists.txt

# INTEL_CUSTOMIZATION
  Vecopt/VPODriver.cpp
#end INTEL_CUSTOMIZATION

  Paropt/VPOParopt.cpp
  Paropt/VPOParoptAtomics.cpp
  Paropt/VPOParoptModuleTransform.cpp
  Paropt/VPOParoptPrepare.cpp
  Paropt/VPOParoptTpv.cpp
  Paropt/VPOParoptTarget.cpp
  Paropt/VPOParoptTask.cpp
  Paropt/VPOParoptTransform.cpp
  Paropt/VPOParoptUtils.cpp
)

add_dependencies(LLVMIntel_VPOTransforms intrinsics_gen intel_directives_gen)

# end INTEL_COLLAB
