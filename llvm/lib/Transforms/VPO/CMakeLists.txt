# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
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
  Paropt/Intel_VPOParoptOptimizeDataSharing.cpp
  Paropt/Intel_VPOParoptSharedPrivatization.cpp
  Paropt/Intel_VPOParoptTargetInline.cpp
  Paropt/Intel_VPOParoptApplyConfig.cpp
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

get_property(LLVMGenXIntrinsics_SOURCE_DIR GLOBAL
             PROPERTY LLVMGenXIntrinsics_SOURCE_PROP)
get_property(LLVMGenXIntrinsics_BINARY_DIR GLOBAL
             PROPERTY LLVMGenXIntrinsics_BINARY_PROP)

include_directories(
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include)

add_llvm_component_library(LLVMVPOTransforms
  VPOTransform.cpp

  Utils/CFGRestructuring.cpp
  Utils/CFGSimplify.cpp
  Utils/VPORenameOperands.cpp
  Utils/VPORestoreOperands.cpp
  Utils/MultiVersioning.cpp
  Utils/ParSecTrans.cpp
# Utils/IntrinsicUtils.cpp // moved to Transforms/Utils/CMakeLists.txt

  Paropt/VPOParopt.cpp
  Paropt/VPOParoptAtomics.cpp
  Paropt/VPOParoptGuardMemoryMotion.cpp
  Paropt/VPOParoptLoopCollapse.cpp
  Paropt/VPOParoptLoopTransform.cpp
  Paropt/VPOParoptLowerSimd.cpp
  Paropt/VPOParoptModuleTransform.cpp
  Paropt/VPOParoptPrepare.cpp
  Paropt/VPOParoptTpv.cpp
  Paropt/VPOParoptTarget.cpp
  Paropt/VPOParoptTask.cpp
  Paropt/VPOParoptTileLoops.cpp
  Paropt/VPOParoptTransform.cpp
  Paropt/VPOParoptUtils.cpp

# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION

  ADDITIONAL_HEADER_DIRS
  ${LLVMGenXIntrinsics_SOURCE_DIR}/GenXIntrinsics/include
  ${LLVMGenXIntrinsics_BINARY_DIR}/GenXIntrinsics/include

  DEPENDS
  intrinsics_gen
  directives_gen
  LLVMDemangle
  LLVMGenXIntrinsics

  LINK_LIBS
  LLVMDemangle
  LLVMGenXIntrinsics

  LINK_COMPONENTS
  Analysis
  Core
# INTEL_CUSTOMIZATION
  Intel_LoopAnalysis
  Intel_LoopTransforms
# end INTEL_CUSTOMIZATION
  Scalar
  Support
  TransformUtils
  VPOAnalysis
)
if(INTEL_CUSTOMIZATION)
# INTEL_FEATURE_SW_DTRANS
  is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
  if (p)
    target_link_libraries(LLVMVPOTransforms PRIVATE LLVMIntel_DTransAnalysis)
  endif()
# end INTEL_FEATURE_SW_DTRANS
endif(INTEL_CUSTOMIZATION)
# end INTEL_COLLAB
