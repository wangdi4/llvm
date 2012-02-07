; RUN: llc -mcpu=corei7-avx -mattr=-avx < %p/../../../llvm/test/CodeGen/X86/vec_clear.ll 
; RUN: llc -mcpu=corei7-avx < %p/../../../llvm/test/CodeGen/X86/vec_clear.ll 

