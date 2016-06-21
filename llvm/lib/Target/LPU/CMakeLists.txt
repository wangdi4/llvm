set(LLVM_TARGET_DEFINITIONS LPU.td)

tablegen(LLVM LPUGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM LPUGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LPUGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM LPUGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM LPUGenCallingConv.inc -gen-callingconv)
tablegen(LLVM LPUGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(LPUCommonTableGen)

add_llvm_target(LPUCodeGen
  LPUISelDAGToDAG.cpp
  LPUISelLowering.cpp
  LPUInstrInfo.cpp
  LPUAllocUnitPass.cpp
  LPUConvertControlPass.cpp
  LPUFrameLowering.cpp
  LPUCvtCFDFPass.cpp
  LPUStatistics.cpp
  LPUOptDFPass.cpp
  LPUMachineFunctionInfo.cpp
  LPURegisterInfo.cpp
  LPUSubtarget.cpp
  LPUTargetMachine.cpp
  LPUSelectionDAGInfo.cpp
  LPUAsmPrinter.cpp
  LPUMCInstLower.cpp
  LPUIfConversion.cpp
  LPUTargetTransformInfo.cpp
  LPULicAllocation.cpp
  MachineCDG.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
