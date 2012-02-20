target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"
; XFAIL: win32
; // (ticket number: CSSD100007402)
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s

define void @testExtractElementLoadStore(<2 x double>* %p){
  %v = load <2 x double>* %p
  %vi = insertelement <2 x double> undef, double 32.5, i32 1
  %vfinal = shufflevector <2 x double> %v, <2 x double> %vi, <2 x i32> <i32 0, i32 3>
; CHECK: movq {{%[a-z0-9]+}}, 8(%rdi)
  store <2 x double> %vfinal, <2 x double>* %p, align 16
  ret void
}
