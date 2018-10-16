# INTEL_COLLAB
# INTEL_CUSTOMIZATION
# Placeholder list for INTEL_CUSTOMIZATION files.
set(INTEL_SOURCE_FILES
  )
# TODO (vzakhari 9/27/2018): there are better ways to having
# CSA-specific VPO transformations isolated from llvm repository:
# 1. We can build VPOParoptTransformCSA.cpp into a separate LLVM
#    component library and link it into VPO transform library
#    for CSA.  Currently VPOParoptTransformCSA.cpp has the following
#    dependencies on VPO transform library: llvm::vpo::WRegionNode::getName,
#    getOmpCanonicalInductionVariable, canHaveLastprivate,
#    getOmpCanonicalInductionVariable, getOmpLoopUpperBound.
#    To avoid cyclic library dependencies, we may want to extract
#    these methods into a separate VPO library (say VPO Util)
#    and then have VPOTransform depend on VPOUtil and VPOCSATransform.
# 2. Another idea is to abstract this CSA specific transformation
#    via TTI, allowing each target to implement their own
#    specific way of handling VPO constructs.
set(INTEL_CSA_SOURCE_FILES
  ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/CSA/lib/Transforms/Intel_VPO/Paropt/VPOParoptTransformCSA.cpp
  )

set(INTEL_SOURCE_FILES_TO_BUILD)
if (INTEL_CUSTOMIZATION)
  if(";${LLVM_INTEL_FEATURES};" MATCHES ";CSA;")
    list(APPEND INTEL_SOURCE_FILES ${INTEL_CSA_SOURCE_FILES})
  else()
    list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_CSA_SOURCE_FILES})
  endif()

  set(INTEL_SOURCE_FILES_TO_BUILD ${INTEL_SOURCE_FILES})
else()
  list(APPEND LLVM_OPTIONAL_SOURCES
    ${INTEL_SOURCE_FILES}
    ${INTEL_CSA_SOURCE_FILES}
    )
endif()
# end INTEL_CUSTOMIZATION

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
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION
  Paropt/VPOParoptUtils.cpp
)

add_dependencies(LLVMIntel_VPOTransforms intrinsics_gen intel_directives_gen)

# end INTEL_COLLAB
