; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define void @and1(<16 x i64> %a, <16 x i64> %b, <16 x i64>* %s) nounwind readnone ssp {
entry:
; CHECK: vpandq
; CHECK: vpandq
  %and = and <16 x i64> %a, %b
  store <16 x i64> %and, <16 x i64>* %s
  ret void
}
