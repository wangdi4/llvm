; RUN: llc -mcpu=sandybridge -march=x86-64 < %p/../../llvm/CodeGen/X86/vector-intrinsics.ll | grep call | count 43
; This expected failure is mapped to CQ# CSSD100005615

