set(LLVM_TARGET_DEFINITIONS CSA.td)

tablegen(LLVM CSAGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM CSAGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CSAGenCSAOpInfo.inc -gen-csa-op-size)
tablegen(LLVM CSAGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CSAGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CSAGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM CSAGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CSAGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(CSACommonTableGen)

add_llvm_target(CSACodeGen
  CSAAllocUnitPass.cpp
  CSAAsmPrinter.cpp
  CSAAsmWrapOstream.cpp
  CSACvtCFDFPass.cpp
  CSADataflowCanonicalization.cpp
  CSADeadInstructionElim.cpp
  CSAExpandInlineAsm.cpp
  CSAFortranIntrinsics.cpp
  CSAFrameLowering.cpp
  CSAIROpt.cpp
  CSAISelDAGToDAG.cpp
  CSAISelLowering.cpp
  CSAInnerLoopPrep.cpp
  CSAInstBuilder.cpp
  CSAInstrInfo.cpp
  CSAIntrinsicCleaner.cpp
  CSALicGrouping.cpp
  CSALoopInfo.cpp
  CSALoopIntrinsicExpander.cpp
  CSALowerAggrCopies.cpp
  CSAMCInstLower.cpp
  CSAMachineFunctionInfo.cpp
  CSAMemopOrdering.cpp
  CSAMultiSeqPass.cpp
  CSANameLICs.cpp
  CSANormalizeDebugPass.cpp
  CSAOptDFPass.cpp
  CSAReassocReduc.cpp
  CSARedundantMovElim.cpp
  CSARegisterInfo.cpp
  CSAReplaceAllocaWithMalloc.cpp
  CSASelectionDAGInfo.cpp
  CSASeqOpt.cpp
  CSAStatistics.cpp
  CSAStreamingMemoryConversion.cpp
  CSAStreamingMemoryPrep.cpp
  CSASubtarget.cpp
  CSATargetMachine.cpp
  MachineCDG.cpp
  CSAUtils.cpp
  CSAProcedureCalls.cpp
  CSARASReplayableLoads.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(FixupOmpEntries)
