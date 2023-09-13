; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx512f | FileCheck %s

; The test checks that SLP patterns based on constant and splat vectors
; formations are recognized by VPlan SLP detector.

; CHECK: Cost decrease due to SLP breaking heuristic is 5
; CHECK: Cost decrease due to SLP breaking heuristic is 1

target triple = "x86_64-unknown-linux-gnu"

%struct.rgb_t = type { i32, i32, i32 }

@a = global [10240 x %struct.rgb_t] zeroinitializer, align 16
@b = global [10240 x %struct.rgb_t] zeroinitializer, align 16

define void @foo2_consts() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %r3 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = load i32, ptr %r3, align 16, !tbaa !3
  %add = add nsw i32 %1, %0
; +1 in lane 0
  %add2 = add nsw i32 %add, 1
  store i32 %add2, ptr %r3, align 16, !tbaa !3
  %g = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %g, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = load i32, ptr %g8, align 4, !tbaa !9
  %add9 = add nsw i32 %3, %2
; -1 in lane 1
  %add10 = add nsw i32 %add9, -1
  store i32 %add10, ptr %g8, align 4, !tbaa !9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo2_splat() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %g0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
; load %r in lane 0
  store i32 %0, ptr %g0, align 16, !tbaa !3
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
; load %r in lane 1
  store i32 %0, ptr %g4, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
; load %r in lane 2
  store i32 %0, ptr %g8, align 8, !tbaa !10
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
; end INTEL_FEATURE_SW_ADVANCED
