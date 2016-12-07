set(LLVM_TARGET_DEFINITIONS LPU.td)

tablegen(LLVM LPUGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM LPUGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LPUGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM LPUGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM LPUGenCallingConv.inc -gen-callingconv)
tablegen(LLVM LPUGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(LPUCommonTableGen)

add_llvm_target(LPUCodeGen
  LPUAllocUnitPass.cpp
  LPUAsmPrinter.cpp
  LPUConvertControlPass.cpp
  LPUCvtCFDFPass.cpp
  LPUDeadInstructionElim.cpp
  LPUFrameLowering.cpp
  LPUISelDAGToDAG.cpp
  LPUISelLowering.cpp
  LPUIfConversion.cpp
  LPUInstrInfo.cpp
  LPULicAllocation.cpp
  LPULowerAggrCopies.cpp
  LPUMCInstLower.cpp
  LPUMachineFunctionInfo.cpp
  LPUOptDFPass.cpp
  LPURedundantMovElim.cpp  
  LPURegisterInfo.cpp
  LPUSelectionDAGInfo.cpp
  LPUStatistics.cpp
  LPUSubtarget.cpp
  LPUTargetMachine.cpp
  LPUTargetTransformInfo.cpp
  MachineCDG.cpp
  CSASetIntrinsicFunctionAttributes.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
