set(LLVM_TARGET_DEFINITIONS LPU.td)

tablegen(LLVM LPUGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM LPUGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM LPUGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM LPUGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM LPUGenCallingConv.inc -gen-callingconv)
tablegen(LLVM LPUGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(LPUCommonTableGen)

add_llvm_target(LPUCodeGen
## LPUBranchSelector.cpp
  LPUISelDAGToDAG.cpp
  LPUISelLowering.cpp
  LPUInstrInfo.cpp
  LPUConvertControlPass.cpp
  LPUFrameLowering.cpp
  LPULICAllocPass.cpp
  LPUMachineFunctionInfo.cpp
  LPUPrologEpilogPass.cpp
  LPURegisterInfo.cpp
  LPUSubtarget.cpp
  LPUTargetMachine.cpp
  LPUSelectionDAGInfo.cpp
  LPUAsmPrinter.cpp
  LPUMCInstLower.cpp
#-  LPUTargetTransformInfo.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
