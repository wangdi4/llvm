; RUN: opt < %s -slp-vectorizer -S -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake -slp-threshold=1000 | FileCheck %s

; This checks that undoMultiNodeReordering() works correctly with siblings nodes.

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
  %A_0 = load i64, i64* %arrayidxA0, align 1, !tbaa !2
  %arrayidxA1 = getelementptr inbounds i64, i64* %src, i64 2
  %A_1 = load i64, i64* %arrayidxA1, align 1, !tbaa !2

  %arrayidxB0 = getelementptr inbounds i64, i64* %src, i64 4
  %B_0 = load i64, i64* %arrayidxB0, align 1, !tbaa !2
  %arrayidxB1 = getelementptr inbounds i64, i64* %src, i64 5
  %B_1 = load i64, i64* %arrayidxB1, align 1, !tbaa !2

  %arrayidxC0 = getelementptr inbounds i64, i64* %src, i64 8
  %C_0 = load i64, i64* %arrayidxC0, align 1, !tbaa !2
  %arrayidxC1 = getelementptr inbounds i64, i64* %src, i64 9
  %C_1 = load i64, i64* %arrayidxC1, align 1, !tbaa !2

  %Sub1_0 = sub i64 %A_0, %B_0
  %Sub1_1 = sub i64 %A_1, %C_1

  %Add0_0 = add i64 %Sub1_0, %C_0
  %Add0_1 = add i64 %Sub1_1, %B_1

; 4 consecutive stores:
  %arrayidx0 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 0
  store i64 %Add0_0, i64* %arrayidx0, align 16, !tbaa !5
  %arrayidx1 = getelementptr inbounds [64 x i64], [64 x i64]* %dst, i64 0, i64 1
  store i64 %Add0_1, i64* %arrayidx1, align 16, !tbaa !5
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


; Make sure that it does not crash. We should have scalar instructions here
; CHECK: [[LOAD1:%.*]] = load i64, i64*
; CHECK: [[LOAD2:%.*]] = load i64, i64*
; CHECK: [[LOAD3:%.*]] = load i64, i64*
; CHECK: [[LOAD4:%.*]] = load i64, i64*
; CHECK: [[LOAD5:%.*]] = load i64, i64*
; CHECK: [[LOAD6:%.*]] = load i64, i64*
; CHECK: store i64 [[WhoCares1:%.*]], i64*
; CHECK: store i64 [[WhoCares2:%.*]], i64*

