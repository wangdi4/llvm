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
set(LLVM_LINK_COMPONENTS
  Support
  )

add_lld_tool(lld
  lld.cpp

  SUPPORT_PLUGINS
  GENERATE_DRIVER
  )
export_executable_symbols_for_plugins(lld)

function(lld_target_link_libraries target type)
  target_link_libraries(${target} ${type} ${ARGN})
  if (TARGET obj.${target})
    target_link_libraries(obj.${target} ${ARGN})
  endif()
endfunction()

lld_target_link_libraries(lld
  PRIVATE
  lldCommon
  lldCOFF
  lldELF
# INTEL_CUSTOMIZATION
# Disable unused functionality to speed up builds.
#  lldMachO
#  lldMinGW
#  lldWasm
# end INTEL_CUSTOMIZATION
  )

install(TARGETS lld
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(NOT LLD_SYMLINKS_TO_CREATE)
  set(LLD_SYMLINKS_TO_CREATE
      lld-link ld.lld ld64.lld wasm-ld)
endif()

if(INTEL_CUSTOMIZATION)
  # All LLD symlinks are installed as part of LLD component.
  # We do not need to deploy darwin and wasm symlinks.
  set(INTEL_LLD_SYMLINKS_TO_DELETE ld64.lld ld64.lld.darwinnew wasm-ld)
  foreach(link ${INTEL_LLD_SYMLINKS_TO_DELETE})
    list(REMOVE_ITEM LLD_SYMLINKS_TO_CREATE ${link})
  endforeach()
endif(INTEL_CUSTOMIZATION)

foreach(link ${LLD_SYMLINKS_TO_CREATE})
  add_lld_symlink(${link} lld)
endforeach()
