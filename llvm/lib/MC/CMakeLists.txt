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
add_llvm_component_library(LLVMMC
  ConstantPools.cpp
  ELFObjectWriter.cpp
  MCAsmBackend.cpp
  MCAsmInfo.cpp
  MCAsmInfoCOFF.cpp
  MCAsmInfoDarwin.cpp
  MCAsmInfoELF.cpp
  MCAsmInfoGOFF.cpp
  MCAsmInfoWasm.cpp
  MCAsmInfoXCOFF.cpp
  MCAsmMacro.cpp
  MCAsmStreamer.cpp
  MCAssembler.cpp
  MCCodeEmitter.cpp
  MCCodeView.cpp
  MCContext.cpp
  MCDwarf.cpp
# INTEL_CUSTOMIZATION
  Intel_MCTrace.cpp
# end INTEL_CUSTOMIZATION
  MCELFObjectTargetWriter.cpp
  MCELFStreamer.cpp
  MCExpr.cpp
  MCFragment.cpp
  MCInst.cpp
  MCInstPrinter.cpp
  MCInstrAnalysis.cpp
  MCInstrDesc.cpp
  MCInstrInfo.cpp
  MCLabel.cpp
  MCLinkerOptimizationHint.cpp
  MCMachOStreamer.cpp
  MCMachObjectTargetWriter.cpp
  MCNullStreamer.cpp
  MCObjectFileInfo.cpp
  MCObjectStreamer.cpp
  MCObjectWriter.cpp
  MCPseudoProbe.cpp
  MCRegisterInfo.cpp
  MCSchedule.cpp
  MCSection.cpp
  MCSectionCOFF.cpp
  MCSectionELF.cpp
  MCSectionMachO.cpp
  MCSectionWasm.cpp
  MCSectionXCOFF.cpp
  MCStreamer.cpp
  MCSPIRVStreamer.cpp
  MCSubtargetInfo.cpp
  MCSymbol.cpp
  MCSymbolELF.cpp
  MCSymbolXCOFF.cpp
  MCTargetOptions.cpp
  MCTargetOptionsCommandFlags.cpp
  MCValue.cpp
  MCWasmObjectTargetWriter.cpp
  MCWasmStreamer.cpp
  MCWin64EH.cpp
  MCWinCOFFStreamer.cpp
  MCWinEH.cpp
  MCXCOFFObjectTargetWriter.cpp
  MCXCOFFStreamer.cpp
  MachObjectWriter.cpp
  SPIRVObjectWriter.cpp
  StringTableBuilder.cpp
  SubtargetFeature.cpp
  TargetRegistry.cpp
  WasmObjectWriter.cpp
  WinCOFFObjectWriter.cpp
  XCOFFObjectWriter.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/MC

  LINK_COMPONENTS
  Support
  BinaryFormat
  DebugInfoCodeView
  )

add_subdirectory(MCParser)
add_subdirectory(MCDisassembler)
