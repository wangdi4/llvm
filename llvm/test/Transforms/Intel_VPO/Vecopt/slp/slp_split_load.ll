; RUN: opt < %s -slp-vectorizer -S -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake | FileCheck %s

; This is a test for Split-Load, which is Variable-Width SLP only for Loads.
; It allows for narrower vector loads compared to the rest of the SLP tree.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@idx = local_unnamed_addr global i64 4, align 4
@src = common global [64 x i64] zeroinitializer, align 1
@res = common local_unnamed_addr global i64 0, align 4

; Function Attrs: noinline nounwind readonly uwtable
define void @foo(i64* nocapture readonly %src, i64 %idx) local_unnamed_addr #0 {

entry:
  %dst = alloca [64 x i64], align 16

; Split load A
  %arrayidxA0 = getelementptr inbounds i64, i64* %src, i64 0
  %A0 = load i64, i64* %arrayidxA0, align 1, !tbaa !2
  %arrayidxA1 = getelementptr inbounds i64, i64* %src, i64 1
  %A1 = load i64, i64* %arrayidxA1, align 1, !tbaa !2

; Split load B
  %arrayidxB0 = getelementptr inbounds i64, i64* %src, i64 8
  %B0 = load i64, i64* %arrayidxB0, align 1, !tbaa !2
  %arrayidxB1 = getelementptr inbounds i64, i64* %src, i64 9
  %B1 = load i64, i64* %arrayidxB1, align 1, !tbaa !2

; 4 consecutive stores:
  %arrayidx0 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 0
  store i64 %A0, i64* %arrayidx0, align 16, !tbaa !5
  %arrayidx1 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 1
  store i64 %A1, i64* %arrayidx1, align 16, !tbaa !5
  %arrayidx2 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 2
  store i64 %B0, i64* %arrayidx2, align 16, !tbaa !5
  %arrayidx3 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 3
  store i64 %B1, i64* %arrayidx3, align 16, !tbaa !5
  ret void
}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA4_A4_j", !7, i64 0}
!7 = !{!"array@_ZTSA4_j", !8, i64 0}
!8 = !{!"int", !3, i64 0}
!9 = !{!8, !8, i64 0}


; Make sure that we have a 4-wide store and 2x 2-wide loads
; CHECK: [[VEC_LOAD1:%.*]] = load <2 x i64>, <2 x i64>*
; CHECK: [[VEC_LOAD2:%.*]] = load <2 x i64>, <2 x i64>*
; CHECK: store <4 x i64> [[WhoCares:%.*]], <4 x i64>*

