; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx512f | FileCheck %s

; The test checks that VPlan SLP detector does not find SLP-able patterns on
; the input code below. Pattern from foo3_neg is not vectorizable due to %arg1
; and/or %arg2 usage. Pattern from foo4_neg is not vectoriable because of
; memory dependency on the fourth memory load, which can not be moved to the
; first load in the pattern.
;
; NOTE: The VPlan SLP algorithm is not able to split the code to distinguish
; vectoriable part. Once (and if) VPlan SLP algorithm would be enhanced to
; catch more SLP possibilities this test may fail expectedly.

; CHECK-NOT: Cost decrease due to SLP breaking heuristic is

target triple = "x86_64-unknown-linux-gnu"

%struct.rgb_t = type { i32, i32, i32, i32 }

@b = global [10240 x %struct.rgb_t] zeroinitializer, align 16
@a = global [10240 x %struct.rgb_t] zeroinitializer, align 16

define void @foo3_neg(i32 %arg1, i32 %arg2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %r3 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = load i32, ptr %r3, align 16, !tbaa !3
  %add = add nsw i32 %1, %arg1
  store i32 %add, ptr %r3, align 16, !tbaa !3
  %g = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %g, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = load i32, ptr %g8, align 4, !tbaa !9
  %add9 = add nsw i32 %3, %2
  store i32 %add9, ptr %g8, align 4, !tbaa !9
  %b = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %4 = load i32, ptr %b, align 8, !tbaa !10
  %b14 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %5 = load i32, ptr %b14, align 8, !tbaa !10
  %add15 = add nsw i32 %arg2, %4
  store i32 %add15, ptr %b14, align 8, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo4_neg() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %r3 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = load i32, ptr %r3, align 16, !tbaa !3
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %r3, align 16, !tbaa !3
  %g = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %g, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = load i32, ptr %g8, align 4, !tbaa !9
  %add9 = add nsw i32 %3, %2
  store i32 %add9, ptr %g8, align 4, !tbaa !9
  %b = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %4 = load i32, ptr %b, align 8, !tbaa !10
  %b14 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %5 = load i32, ptr %b14, align 8, !tbaa !10
  %add15 = add nsw i32 %5, %4
  store i32 %add15, ptr %b14, align 8, !tbaa !10
  %t = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !14
  %6 = load i32, ptr %t, align 4, !tbaa !14
  %t20 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !14
  %7 = load i32, ptr %t20, align 4, !tbaa !14
  %add21 = add nsw i32 %7, %6
  store i32 %add21, ptr %t20, align 4, !tbaa !14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

!3 = !{!4, !6, i64 0}
!4 = !{!"array@_ZTSA10240_5rgb_t", !5, i64 0}
!5 = !{!"struct@", !6, i64 0, !6, i64 4, !6, i64 8, !6, i64 12}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!4, !6, i64 4}
!10 = !{!4, !6, i64 8}
!14 = !{!4, !6, i64 8}
; end INTEL_FEATURE_SW_ADVANCED
