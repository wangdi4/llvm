#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#

set(LLVM_TARGET_DEFINITIONS CSA.td)

tablegen(LLVM CSAGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM CSAGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CSAGenCSAOpInfo.inc -gen-csa-op-size)
tablegen(LLVM CSAGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CSAGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CSAGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM CSAGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CSAGenMAPatterns.inc -gen-ma-patterns)
tablegen(LLVM CSAGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(CSACommonTableGen)

add_llvm_target(CSACodeGen
  CSAAllocUnitPass.cpp
  CSAAsmPrinter.cpp
  CSAAsmWrapOstream.cpp
  CSABackedgeVerifier.cpp
  CSACacheLocalizer.cpp
  CSACreateSelfContainedGraph.cpp
  CSACvtCFDFPass.cpp
  CSADataflowCanonicalization.cpp
  CSADataflowVerifier.cpp
  CSADeadInstructionElim.cpp
  CSAExpandInlineAsm.cpp
  CSAFMA.cpp
  CSAFortranIntrinsics.cpp
  CSAFrameLowering.cpp
  CSAIROpt.cpp
  CSAISelDAGToDAG.cpp
  CSAISelLowering.cpp
  CSALoopPrep.cpp
  CSAInnerLoopPrep.cpp
  CSAInstBuilder.cpp
  CSAInstrInfo.cpp
  CSAIntrinsicCleaner.cpp
  CSALicGrouping.cpp
  CSALoopInfo.cpp
  CSALoopIntrinsicExpander.cpp
  CSALowerLoopIdioms.cpp
  CSALowerScratchpads.cpp
  CSAMCInstLower.cpp
  CSAMachineFunctionInfo.cpp
  CSAMultiSeqPass.cpp
  CSANameLICs.cpp
  CSANormalizeDebugPass.cpp
  CSAOptDFPass.cpp
  CSAReassocReduc.cpp
  CSARegisterInfo.cpp
  CSAReplaceAllocaWithMalloc.cpp
  CSASelectionDAGInfo.cpp
  CSASeqOpt.cpp
  CSAStatistics.cpp
  CSAStreamingMemoryConversion.cpp
  CSASubtarget.cpp
  CSATargetMachine.cpp
  CSATargetTransformInfo.cpp
  MachineCDG.cpp
  CSAUtils.cpp
  CSAProcedureCalls.cpp
  CSARASReplayableLoads.cpp
  CSASeqotToSeqOptimization.cpp
  MemopOrdering/CSALinearMemopOrdering.cpp
  MemopOrdering/CSAMemopOrdering.cpp
  MemopOrdering/CSAMemopOrderingBase.cpp
  MemopOrdering/CSAMemopOrderingPassScheduling.cpp
  MemopOrdering/CSARaceModeMemopOrdering.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
