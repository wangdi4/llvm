; RUN: llc -mcpu=sandybridge -mattr=-avx < %p/../../llvm/CodeGen/X86/vec_clear.ll 
; RUN: llc -mcpu=sandybridge < %p/../../llvm/CodeGen/X86/vec_clear.ll 

