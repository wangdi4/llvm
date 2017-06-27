set(LLVM_TARGET_DEFINITIONS CSA.td)

tablegen(LLVM CSAGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM CSAGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CSAGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CSAGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CSAGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM CSAGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CSAGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(CSACommonTableGen)

add_llvm_target(CSACodeGen
  CSAAllocUnitPass.cpp
  CSAAsmPrinter.cpp
  CSAConvertControlPass.cpp
  CSACvtCFDFPass.cpp
  CSADeadInstructionElim.cpp
  CSAFrameLowering.cpp
  CSAISelDAGToDAG.cpp
  CSAISelLowering.cpp
  CSAIfConversion.cpp
  CSAInstrInfo.cpp
  CSALicAllocation.cpp
  CSALowerAggrCopies.cpp
  CSAMCInstLower.cpp
  CSAMachineFunctionInfo.cpp
  CSAOptDFPass.cpp
  CSADFParLoopPass.cpp
  CSARedundantMovElim.cpp
  CSARegisterInfo.cpp
  CSASelectionDAGInfo.cpp
  CSAStatistics.cpp
  CSASubtarget.cpp
  CSATargetMachine.cpp
  CSATargetTransformInfo.cpp
  MachineCDG.cpp
  CSAExpandInlineAsm.cpp
  CSAMemopOrdering.cpp
  CSAFortranIntrinsics.cpp
  CSANormalizeDebugPass.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
