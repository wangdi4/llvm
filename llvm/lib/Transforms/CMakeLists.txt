# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
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
add_subdirectory(Utils)
add_subdirectory(Instrumentation)
add_subdirectory(AggressiveInstCombine)
add_subdirectory(InstCombine)
add_subdirectory(Scalar)
add_subdirectory(IPO)
add_subdirectory(Vectorize)
add_subdirectory(Hello)
add_subdirectory(ObjCARC)
if (INTEL_CUSTOMIZATION)
add_subdirectory(Intel_DPCPPKernelTransforms)
add_subdirectory(Intel_LoopTransforms)
add_subdirectory(Intel_MapIntrinToIml)
add_subdirectory(Intel_OpenCLTransforms)
endif(INTEL_CUSTOMIZATION)
if (INTEL_COLLAB)
add_subdirectory(VPO)
endif(INTEL_COLLAB)
add_subdirectory(Coroutines)
add_subdirectory(CFGuard)
