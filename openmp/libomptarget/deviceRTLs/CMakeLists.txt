##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# ##===----------------------------------------------------------------------===##
#
# Build a device RTL for each available machine.
#
##===----------------------------------------------------------------------===##

add_subdirectory(nvptx)

#INTEL_CUSTOMIZATION
if(INTEL_CUSTOMIZATION)
  add_subdirectory(nios2)
endif()
#END INTEL_CUSTOMIZATION

if(INTEL_COLLAB)
  add_subdirectory(opencl)
endif(INTEL_COLLAB)
