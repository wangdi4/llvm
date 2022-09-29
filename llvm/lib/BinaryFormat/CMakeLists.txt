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
add_llvm_component_library(LLVMBinaryFormat
  AMDGPUMetadataVerifier.cpp
  COFF.cpp
  Dwarf.cpp
  DXContainer.cpp
  ELF.cpp
# INTEL_CUSTOMIZATION
  Intel_Trace.cpp
# end INTEL_CUSTOMIZATION
  MachO.cpp
  Magic.cpp
  Minidump.cpp
  MsgPackDocument.cpp
  MsgPackDocumentYAML.cpp
  MsgPackReader.cpp
  MsgPackWriter.cpp
  Wasm.cpp
  XCOFF.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/BinaryFormat

  LINK_COMPONENTS
  Support
  )
