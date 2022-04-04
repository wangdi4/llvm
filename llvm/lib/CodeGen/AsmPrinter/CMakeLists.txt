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
add_llvm_component_library(LLVMAsmPrinter
  AccelTable.cpp
  AddressPool.cpp
  AIXException.cpp
  ARMException.cpp
  AsmPrinter.cpp
  AsmPrinterDwarf.cpp
  AsmPrinterInlineAsm.cpp
  DbgEntityHistoryCalculator.cpp
  DebugHandlerBase.cpp
  DebugLocStream.cpp
  DIE.cpp
  DIEHash.cpp
  DwarfCFIException.cpp
  DwarfCompileUnit.cpp
  DwarfDebug.cpp
# INTEL_CUSTOMIZATION
  Intel_TraceBackDebug.cpp
# end INTEL_CUSTOMIZATION
  DwarfExpression.cpp
  DwarfFile.cpp
  DwarfStringPool.cpp
  DwarfUnit.cpp
  EHStreamer.cpp
  ErlangGCPrinter.cpp
  OcamlGCPrinter.cpp
  PseudoProbePrinter.cpp
  WinCFGuard.cpp
  WinException.cpp
  CodeViewDebug.cpp
# INTEL_CUSTOMIZATION
  intel/Intel_AsmOptReport.cpp
# end INTEL_CUSTOMIZATION
  WasmException.cpp

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  BinaryFormat
  CodeGen
  Core
  DebugInfoCodeView
  DebugInfoDWARF
  DebugInfoMSF
# INTEL_CUSTOMIZATION
  DebugInfoTraceBack
# end INTEL_CUSTOMIZATION
  MC
  MCParser
  Remarks
  Support
  Target
  )

