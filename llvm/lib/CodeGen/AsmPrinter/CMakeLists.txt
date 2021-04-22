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
#if INTEL_CUSTOMIZATION
  Intel_TraceBackDebug.cpp
#end INTEL_CUSTOMIZATION
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
#***INTEL
  intel/STIDebug.cpp
  intel/STIIR.cpp
  intel/pdbInterface.cpp
#if INTEL_CUSTOMIZATION
  intel/Intel_AsmOptReport.cpp
#end INTEL_CUSTOMIZATION
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

if(INTEL_CUSTOMIZATION)
  target_link_libraries(LLVMAsmPrinter PRIVATE LLVMIntel_OptReport)
  target_link_libraries(LLVMAsmPrinter PRIVATE LLVMIntel_LoopOptReportSupport)
endif(INTEL_CUSTOMIZATION)

