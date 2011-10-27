; RUN: llc -mcpu=sandybridge < %p/../../llvm/CodeGen/X86/widen_select-1.ll 
; CHECK: jne
