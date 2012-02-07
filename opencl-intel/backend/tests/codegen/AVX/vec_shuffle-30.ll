; RUN: llc -mcpu=corei7-avx -mattr=-avx < %p/../../../llvm/test/CodeGen/X86/vec_shuffle-30.ll 
; RUN: llc -mcpu=corei7-avx < %p/../../../llvm/test/CodeGen/X86/vec_shuffle-30.ll 
; This expected failure is mapped to CQ# CSSD100005615
