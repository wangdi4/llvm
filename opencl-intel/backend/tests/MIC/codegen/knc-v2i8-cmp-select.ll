; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test(<2 x i8>* %srcA, <2 x i8>* %srcB, <2 x i8>* %dst) {
; CHECK: cmpb
; CHECK: setb [[R1:%[a-z0-9]+]]
; CHECK: subb [[R1]], {{%[a-z0-9]+}}
  %arrayidx = getelementptr inbounds <2 x i8>* %srcA, i64 0
  %1 = load <2 x i8>* %arrayidx
  %arrayidx2 = getelementptr inbounds <2 x i8>* %srcB, i64 0
  %2 = load <2 x i8>* %arrayidx2
  %cmp1 = icmp eq <2 x i8> %1, zeroinitializer
  %cmp2 = icmp eq <2 x i8> %2, zeroinitializer
  %select1 = select <2 x i1> %cmp1, <2 x i8> zeroinitializer, <2 x i8> <i8 -1, i8 -1>
  %select2 = select <2 x i1> %cmp2, <2 x i8> zeroinitializer, <2 x i8> %select1
  %arrayidx4 = getelementptr inbounds <2 x i8>* %dst, i64 0
  store <2 x i8> %select2, <2 x i8>* %arrayidx4
  ret void
}
