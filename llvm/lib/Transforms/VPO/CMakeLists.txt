# INTEL_COLLAB

# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
# Add custom Intel files below.  Files listed here will be included in
# the INTEL_CUSTOMIZATION build.
  ../Intel_VPO/Vecopt/VPODirectiveCleanup.cpp
  Paropt/Intel_VPOParoptFortranUtils.cpp
  )

# INTEL_FEATURE_CSA
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
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  FEATURE ${LLVM_INTELFEATURE_PREFIX}_CSA
  ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/CSA/lib/Transforms/Intel_VPO/Paropt/VPOParoptTransformCSA.cpp
  )
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION

add_llvm_library(LLVMVPOTransforms
  VPOTransform.cpp

  Utils/CFGRestructuring.cpp
  Utils/VPORestoreOperands.cpp
  Utils/MultiVersioning.cpp
  Utils/ParSecTrans.cpp
# Utils/IntrinsicUtils.cpp // moved to Transforms/Utils/CMakeLists.txt

  Paropt/VPOParopt.cpp
  Paropt/VPOParoptAtomics.cpp
  Paropt/VPOParoptModuleTransform.cpp
  Paropt/VPOParoptPrepare.cpp
  Paropt/VPOParoptTpv.cpp
  Paropt/VPOParoptTarget.cpp
  Paropt/VPOParoptTask.cpp
  Paropt/VPOParoptTransform.cpp
  Paropt/VPOParoptUtils.cpp
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION
)

add_dependencies(LLVMVPOTransforms intrinsics_gen directives_gen)

# end INTEL_COLLAB
