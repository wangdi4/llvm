# INTEL_COLLAB
#
#    Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

add_llvm_library(LLVMIntel_VPOAnalysis
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
)

add_dependencies(LLVMIntel_VPOAnalysis intrinsics_gen intel_directives_gen)

# end INTEL_COLLAB
