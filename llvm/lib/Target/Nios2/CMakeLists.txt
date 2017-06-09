set(LLVM_TARGET_DEFINITIONS Nios2Asm.td)
tablegen(LLVM Nios2GenAsmMatcher.inc -gen-asm-matcher)

set(LLVM_TARGET_DEFINITIONS Nios2Other.td)

# Generate Nios2GenRegisterInfo.inc and Nios2GenInstrInfo.inc which included by 
#  your hand code C++ files. 
# Nios2GenRegisterInfo.inc came from Nios2RegisterInfo.td, Nios2GenInstrInfo.inc 
#  came from Nios2InstrInfo.td.
tablegen(LLVM Nios2GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM Nios2GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Nios2GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM Nios2GenCodeEmitter.inc -gen-emitter)
tablegen(LLVM Nios2GenMCCodeEmitter.inc -gen-emitter)

tablegen(LLVM Nios2GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM Nios2GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM Nios2GenCallingConv.inc -gen-callingconv)
tablegen(LLVM Nios2GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM Nios2GenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM Nios2GenRegisterBank.inc -gen-register-bank)
tablegen(LLVM Nios2GenGlobalISel.inc -gen-global-isel)

# Nios2CommonTableGen must be defined
add_public_tablegen_target(Nios2CommonTableGen)

# Nios2CodeGen should match with LLVMBuild.txt Nios2CodeGen
add_llvm_target(Nios2CodeGen
  Nios2AnalyzeImmediate.cpp
  Nios2AsmPrinter.cpp
  Nios2MCInstLower.cpp
  Nios2DelaySlotFiller.cpp
  Nios2DelUselessJMP.cpp
  Nios2EmitGPRestore.cpp
  Nios2FrameLowering.cpp
  Nios2InstrInfo.cpp
  Nios2ISelLowering.cpp
  Nios2MachineFunction.cpp
  Nios2RegisterInfo.cpp
  Nios2SEFrameLowering.cpp
  Nios2SEInstrInfo.cpp
  Nios2SEISelLowering.cpp
  Nios2SERegisterInfo.cpp
  Nios2Subtarget.cpp
  Nios2TargetObjectFile.cpp
  Nios2ISelDAGToDAG.cpp
  Nios2SEISelDAGToDAG.cpp
  Nios2TargetMachine.cpp
  Nios2CallLowering.cpp
  Nios2InstructionSelector.cpp
  Nios2LegalizerInfo.cpp
  Nios2RegisterBankInfo.cpp

  Nios2LowerAllocaPass.cpp
  Nios2MidEndUtil.cpp
  )

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(InstPrinter)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)
