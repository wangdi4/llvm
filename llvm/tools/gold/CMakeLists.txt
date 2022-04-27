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
set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/gold.exports)

if( LLVM_ENABLE_PIC AND LLVM_BINUTILS_INCDIR )
  include_directories( ${LLVM_BINUTILS_INCDIR} )

  set(LLVM_LINK_COMPONENTS
     ${LLVM_TARGETS_TO_BUILD}
     Linker
     LTO
     BitWriter
     IPO
     )

  add_llvm_library(LLVMgold MODULE
    gold-plugin.cpp
    )

  if(INTEL_CUSTOMIZATION)
    # CMPLRLLVM-20054: icx-lto is a release name for LLVMgold plugin.
    #
    # LLVMgold is only used for LIT testing. We have to keep LLVMgold,
    # since we do not want to modify LIT tests that hard-code LLVMgold
    # plugin name, and deal with conflicts and test fails after llorg
    # pulldowns.
    add_llvm_library(icx-lto MODULE
      gold-plugin.cpp
      )
  endif(INTEL_CUSTOMIZATION)
endif()
