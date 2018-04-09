##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
# ##===----------------------------------------------------------------------===##
#
# Build a device RTL for each available machine available.
#
##===----------------------------------------------------------------------===##

add_subdirectory(nvptx)

#INTEL_CUSTOMIZATION
if(INTEL_CUSTOMIZATION)
  add_subdirectory(nios2)
endif()
#END INTEL_CUSTOMIZATION
