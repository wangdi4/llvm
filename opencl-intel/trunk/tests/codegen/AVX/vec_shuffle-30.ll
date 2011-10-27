; RUN: llc -mcpu=sandybridge -mattr=-avx < %p/../../llvm/CodeGen/X86/vec_shuffle-30.ll 
; RUN: llc -mcpu=sandybridge < %p/../../llvm/CodeGen/X86/vec_shuffle-30.ll 
; This expected failure is mapped to CQ# CSSD100005615
