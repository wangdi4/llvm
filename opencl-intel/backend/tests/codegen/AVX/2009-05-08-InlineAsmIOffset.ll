; RUN: llc < %p/../../llvm/CodeGen/X86/2009-05-08-InlineAsmIOffset.ll -mcpu=sandybridge -relocation-model=static > %t
; RUN: grep "1: ._pv_cpu_ops+8" %t
; RUN: grep "2: ._G" %t
; PR4152
