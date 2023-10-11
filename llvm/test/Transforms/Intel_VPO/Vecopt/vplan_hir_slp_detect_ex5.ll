; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx2 | FileCheck %s

; This test is designed to check SLP detection code ability to find splat vector
; of length 2 within 4 elements array. Currently the result is observable in
; debug dumps only (-debug-only=intel-vplan-slp -vplan-slp-report-detail-level=3)
; as SLP detection code does not support vector length reduction.
; The test CHECK line contains the check that SLP does not trigger which does not
; provide sufficient check for splat vector detection. The check has to be reverted
; once SLP detection supports vector length reduction.

; CHECK-NOT: Cost decrease due to SLP breaking heuristic

target triple = "x86_64-unknown-linux-gnu"

%struct.rgb_t = type { i32, i32, i32, i32 }

@a = global [10240 x %struct.rgb_t] zeroinitializer, align 16
@b = global [10240 x %struct.rgb_t] zeroinitializer, align 16

define void @foo1_splat() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r, align 16, !tbaa !3
  %g0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
; load %r + 1 in lane 0
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %g0, align 16, !tbaa !3
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
; load %r in lane 1
  store i32 %0, ptr %g4, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
; load %r in lane 2
  store i32 %0, ptr %g8, align 8, !tbaa !10
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
; load %r * 2 in lane 3
  %dec = mul nsw i32 %0, 2
  store i32 %dec, ptr %g12, align 8, !tbaa !11
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
; load %r + 1 in lane 0
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %g0, align 16, !tbaa !3
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
; load %r in lane 1
  store i32 %0, ptr %g4, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
; load %r * 2 in lane 2
  %dec = mul nsw i32 %0, 2
  store i32 %dec, ptr %g8, align 8, !tbaa !10
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
; load %r in lane 3
  store i32 %0, ptr %g12, align 8, !tbaa !11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo3_splat() {
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
; load %r + 1 in lane 1
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %g4, align 4, !tbaa !9
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
; load %r in lane 2
  store i32 %0, ptr %g8, align 8, !tbaa !10
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
; load %r * 2 in lane 3
  %dec = mul nsw i32 %0, 2
  store i32 %dec, ptr %g12, align 8, !tbaa !11
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
!11 = !{!4, !6, i64 12}
; end INTEL_FEATURE_SW_ADVANCED
