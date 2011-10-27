; RUN: llc -mcpu=sandybridge < %p/../../llvm/CodeGen/X86/2008-09-05-sinttofp-2xi32.ll 
; XFAIL: *
; This expected failure is mapped to CQ# CSSD100005615
