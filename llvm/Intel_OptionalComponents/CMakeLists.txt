#
#    Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

if (INTEL_INCLUDE_DTRANS)
  add_subdirectory(DTrans)
endif()

# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if(p)
  add_subdirectory(CSA)
endif()
# end INTEL_FEATURE_CSA
