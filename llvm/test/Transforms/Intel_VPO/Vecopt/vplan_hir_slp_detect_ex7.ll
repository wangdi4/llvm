; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx | FileCheck %s

; This example exercises cases when 'sub' and 'add' operations are mixed.
; VSLP is expected to catch this case and suggest profitable SLP vectorization.
;
; CHECK: Cost decrease due to SLP breaking heuristic is 9
; CHECK: Cost decrease due to SLP breaking heuristic is 9
; CHECK: Cost decrease due to SLP breaking heuristic is 9

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ai = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 64
@bi = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 64
@ci = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 64

define void @foo1() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 16, !tbaa !3
  %arrayidx3 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx3, align 16, !tbaa !3
; lane 0: sub
  %sub4 = sub nsw i32 %0, %1
  %arrayidx7 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %indvars.iv, !intel-tbaa !3
  store i32 %sub4, ptr %arrayidx7, align 16, !tbaa !3
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %2, !intel-tbaa !3
  %3 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %2, !intel-tbaa !3
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !3
; lane 1: sub
  %sub14 = sub nsw i32 %3, %4
  %arrayidx17 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %2, !intel-tbaa !3
  store i32 %sub14, ptr %arrayidx17, align 4, !tbaa !3
  %5 = or i64 %indvars.iv, 2
  %arrayidx20 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %5, !intel-tbaa !3
  %6 = load i32, ptr %arrayidx20, align 8, !tbaa !3
  %arrayidx23 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %5, !intel-tbaa !3
  %7 = load i32, ptr %arrayidx23, align 8, !tbaa !3
; lane 2: add
  %add24 = add nsw i32 %6, %7
  %arrayidx27 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %5, !intel-tbaa !3
  store i32 %add24, ptr %arrayidx27, align 8, !tbaa !3
  %8 = or i64 %indvars.iv, 3
  %arrayidx30 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %8, !intel-tbaa !3
  %9 = load i32, ptr %arrayidx30, align 4, !tbaa !3
  %arrayidx33 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %8, !intel-tbaa !3
  %10 = load i32, ptr %arrayidx33, align 4, !tbaa !3
; lane 3: add
  %add34 = add nsw i32 %9, %10
  %arrayidx37 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %8, !intel-tbaa !3
  store i32 %add34, ptr %arrayidx37, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv, 1020
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo2() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 16, !tbaa !3
  %arrayidx3 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx3, align 16, !tbaa !3
; lane 0: sub
  %sub4 = sub nsw i32 %0, %1
  %arrayidx7 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %indvars.iv, !intel-tbaa !3
  store i32 %sub4, ptr %arrayidx7, align 16, !tbaa !3
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %2, !intel-tbaa !3
  %3 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %2, !intel-tbaa !3
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !3
; lane 1: add
  %add14 = add nsw i32 %3, %4
  %arrayidx17 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %2, !intel-tbaa !3
  store i32 %add14, ptr %arrayidx17, align 4, !tbaa !3
  %5 = or i64 %indvars.iv, 2
  %arrayidx20 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %5, !intel-tbaa !3
  %6 = load i32, ptr %arrayidx20, align 8, !tbaa !3
  %arrayidx23 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %5, !intel-tbaa !3
  %7 = load i32, ptr %arrayidx23, align 8, !tbaa !3
; lane 2: sub
  %sub24 = sub nsw i32 %6, %7
  %arrayidx27 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %5, !intel-tbaa !3
  store i32 %sub24, ptr %arrayidx27, align 8, !tbaa !3
  %8 = or i64 %indvars.iv, 3
  %arrayidx30 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %8, !intel-tbaa !3
  %9 = load i32, ptr %arrayidx30, align 4, !tbaa !3
  %arrayidx33 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %8, !intel-tbaa !3
  %10 = load i32, ptr %arrayidx33, align 4, !tbaa !3
; lane 3: add
  %add34 = add nsw i32 %9, %10
  %arrayidx37 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %8, !intel-tbaa !3
  store i32 %add34, ptr %arrayidx37, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv, 1020
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo3() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %0 = load i32, ptr %arrayidx, align 16, !tbaa !3
  %arrayidx3 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %1 = load i32, ptr %arrayidx3, align 16, !tbaa !3
; lane 0: sub
  %sub4 = sub nsw i32 %0, %1
  %arrayidx7 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %indvars.iv, !intel-tbaa !3
  store i32 %sub4, ptr %arrayidx7, align 16, !tbaa !3
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %2, !intel-tbaa !3
  %3 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  %arrayidx13 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %2, !intel-tbaa !3
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !3
; lane 1: add
  %add14 = add nsw i32 %3, %4
  %arrayidx17 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %2, !intel-tbaa !3
  store i32 %add14, ptr %arrayidx17, align 4, !tbaa !3
  %5 = or i64 %indvars.iv, 2
  %arrayidx20 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %5, !intel-tbaa !3
  %6 = load i32, ptr %arrayidx20, align 8, !tbaa !3
  %arrayidx23 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %5, !intel-tbaa !3
  %7 = load i32, ptr %arrayidx23, align 8, !tbaa !3
; lane 2: add
  %add24 = add nsw i32 %6, %7
  %arrayidx27 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %5, !intel-tbaa !3
  store i32 %add24, ptr %arrayidx27, align 8, !tbaa !3
  %8 = or i64 %indvars.iv, 3
  %arrayidx30 = getelementptr inbounds [1024 x i32], ptr @ai, i64 0, i64 %8, !intel-tbaa !3
  %9 = load i32, ptr %arrayidx30, align 4, !tbaa !3
  %arrayidx33 = getelementptr inbounds [1024 x i32], ptr @bi, i64 0, i64 %8, !intel-tbaa !3
  %10 = load i32, ptr %arrayidx33, align 4, !tbaa !3
; lane 3: sub
  %sub34 = sub nsw i32 %9, %10
  %arrayidx37 = getelementptr inbounds [1024 x i32], ptr @ci, i64 0, i64 %8, !intel-tbaa !3
  store i32 %sub34, ptr %arrayidx37, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv, 1020
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA1024_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !12, i64 0}
!11 = !{!"array@_ZTSA1024_f", !12, i64 0}
!12 = !{!"float", !6, i64 0}
; end INTEL_FEATURE_SW_ADVANCED
