; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -mattr=+avx512f | FileCheck %s

; The test checks that SLP patterns in all functions are found to be profitable for
; SSE-able target. These samples target commutativity support within SLP detection code.

; CHECK: Cost decrease due to SLP breaking heuristic is 8
; CHECK: Cost decrease due to SLP breaking heuristic is 9
; CHECK: Cost decrease due to SLP breaking heuristic is 8

target triple = "x86_64-unknown-linux-gnu"

%struct.rgb_t = type { i32, i32, i32, i32 }

@a = global [10240 x %struct.rgb_t] zeroinitializer, align 16
@b = global [10240 x %struct.rgb_t] zeroinitializer, align 16

define void @foo_param(i32 %ex) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r0, align 16, !tbaa !3
  %g0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = add nsw i32 %0, %ex
; *%r0 + %ex in lane 0
  store i32 %1, ptr %g0, align 16, !tbaa !3

  %r4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %r4, align 16, !tbaa !9
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = add nsw i32 %ex, %2
; %ex + *%r4 in lane 1
  store i32 %3, ptr %g4, align 4, !tbaa !9

  %r8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %4 = load i32, ptr %r8, align 16, !tbaa !10
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %5 = add nsw i32 %4, %ex
; *%r8 + %ex in lane 2
  store i32 %5, ptr %g8, align 8, !tbaa !10

  %r12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %6 = load i32, ptr %r12, align 16, !tbaa !11
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %7 = add nsw i32 %ex, %6
; %ex + *%r12 in lane 3
  store i32 %7, ptr %g12, align 8, !tbaa !11

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo_const() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %r0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r0, align 16, !tbaa !3
  %g0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = add nsw i32 %0, 1
; *%r0 + 1 in lane 0
  store i32 %1, ptr %g0, align 16, !tbaa !3

  %r4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %r4, align 16, !tbaa !9
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = add nsw i32 2, %2
; 2 + *%r4 in lane 1
  store i32 %3, ptr %g4, align 4, !tbaa !9

  %r8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %4 = load i32, ptr %r8, align 16, !tbaa !10
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %5 = add nsw i32 %4, 3
; *%r8 + 3 in lane 2
  store i32 %5, ptr %g8, align 8, !tbaa !10

  %r12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %6 = load i32, ptr %r12, align 16, !tbaa !11
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %7 = add nsw i32 4, %6
; 4 + *%r12 in lane 3
  store i32 %7, ptr %g12, align 8, !tbaa !11

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10240
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @foo_splat(ptr %p) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ex = load i32, ptr %p, align 16, !tbaa !6
  %r0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %0 = load i32, ptr %r0, align 16, !tbaa !3
  %g0 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 0, !intel-tbaa !3
  %1 = add nsw i32 %0, %ex
; *%r0 + %ex in lane 0
  store i32 %1, ptr %g0, align 16, !tbaa !3

  %r4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %2 = load i32, ptr %r4, align 16, !tbaa !9
  %g4 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 1, !intel-tbaa !9
  %3 = add nsw i32 %ex, %2
; %0 + *%r4 in lane 1
  store i32 %3, ptr %g4, align 4, !tbaa !9

  %r8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %4 = load i32, ptr %r8, align 16, !tbaa !10
  %g8 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 2, !intel-tbaa !10
  %5 = add nsw i32 %4, %ex
; *%r8 + %ex in lane 2
  store i32 %5, ptr %g8, align 8, !tbaa !10

  %r12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @b, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %6 = load i32, ptr %r12, align 16, !tbaa !11
  %g12 = getelementptr inbounds [10240 x %struct.rgb_t], ptr @a, i64 0, i64 %indvars.iv, i32 3, !intel-tbaa !11
  %7 = add nsw i32 %ex, %6
; %ex + *%r12 in lane 3
  store i32 %7, ptr %g12, align 8, !tbaa !11

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
