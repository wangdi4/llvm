; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec -vplan-cost-model-print-analysis-for-vf=1 -march=+sse -disable-output 2>&1 | FileCheck %s

; The test exersizes VPlan SLP detection code on non-vectorizable types.
; It is checked that VSLP doesn't crash and SLP pattern is not detected.

; CHECK-NOT: Cost decrease due to SLP breaking heuristic

%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16
@pS = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

define void @test() {
alloca:
  br label %bb5

bb5:                                            ; preds = %bb5, %alloca
  %p0 = phi i32 [ 0, %alloca ], [ %add21, %bb5 ]
  %int_sext = sext i32 %p0 to i64
  %pR = getelementptr inbounds [100 x %complex_64bit], ptr @pR, i32 0, i64 %int_sext, !intel-tbaa !3
  %pR_fetch = load %complex_64bit, ptr %pR, !tbaa !3
  %pS = getelementptr inbounds [100 x %complex_64bit], ptr @pS, i32 0, i64 %int_sext, !intel-tbaa !3
  store %complex_64bit %pR_fetch, ptr %pS, !tbaa !3

  %int_sext4 = add nsw i64 %int_sext, 4
  %pR4 = getelementptr inbounds [100 x %complex_64bit], ptr @pR, i32 0, i64 %int_sext4, !intel-tbaa !9
  %pR_fetch4 = load %complex_64bit, ptr %pR, !tbaa !9
  %pS4 = getelementptr inbounds [100 x %complex_64bit], ptr @pS, i32 0, i64 %int_sext4, !intel-tbaa !9
  store %complex_64bit %pR_fetch4, ptr %pS4, !tbaa !9

  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %bb5, label %bb1

bb1:                                              ; preds = %bb5
  ret void
}

!3 = !{!4, !6, i64 0}
!4 = !{!"array@complex_64bit", !5, i64 0}
!5 = !{!"struct@", !6, i64 0, !6, i64 8}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!4, !6, i64 8}
; end INTEL_FEATURE_SW_ADVANCED
