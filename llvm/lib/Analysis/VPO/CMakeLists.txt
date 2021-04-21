# INTEL_COLLAB
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

add_llvm_component_library(LLVMVPOAnalysis
  VPOAnalysis.cpp
  Utils/GeneralUtils.cpp
  # Utils/IntrinsicUtils.cpp             # Moved to lib/Analysis/CMakeLists.txt
  # Utils/OpenMPDirectivesAndClauses.cpp # Moved to lib/Analysis/CMakeLists.txt

  WRegionInfo/WRegionNode.cpp
  WRegionInfo/WRegion.cpp
  WRegionInfo/WRegionClause.cpp
  WRegionInfo/WRegionCollection.cpp
  WRegionInfo/WRegionInfo.cpp
  WRegionInfo/WRegionUtils.cpp
  WRegionInfo/WRegionInfoAnalysis.cpp

  DEPENDS
  intrinsics_gen
  directives_gen

  LINK_COMPONENTS
  Analysis
  Core
# INTEL_CUSTOMIZATION
  Intel_LoopAnalysis
# end INTEL_CUSTOMIZATION
  Scalar
  Support
  TransformUtils
)
# end INTEL_COLLAB
