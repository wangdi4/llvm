# MCTargetDesc/CMakeLists.txt
add_llvm_library(LLVMNios2Desc
  Nios2ABIInfo.cpp
  Nios2MCAsmInfo.cpp
  Nios2AsmBackend.cpp
  Nios2MCCodeEmitter.cpp
  Nios2MCExpr.cpp
  Nios2ELFObjectWriter.cpp
  Nios2TargetStreamer.cpp
  Nios2MCTargetDesc.cpp
  )
