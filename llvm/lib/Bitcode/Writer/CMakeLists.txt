add_llvm_library(LLVMBitWriter
  BitWriter.cpp
  BitcodeWriter.cpp
  BitcodeWriterPass.cpp
  ValueEnumerator.cpp
  LPUSaveRawBC.cpp

  DEPENDS
  intrinsics_gen
  )
