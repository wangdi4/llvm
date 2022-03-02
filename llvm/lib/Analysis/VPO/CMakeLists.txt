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
# INTEL_CUSTOMIZATION
  Intel_VPOParoptConfig.cpp
# end INTEL_CUSTOMIZATION

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
