; RUN: opt < %s -disable-output -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec \
; RUN:     -debug-only=LoopVectorizationPlanner,LoopVectorizationPlanner_peel_tc \
; RUN:     -vplan-enable-peeling=true -vplan-optimize-profitable-peel-check=true \
; RUN:     -vplan-force-vf=4 -vplan-print-after-cfg-merge \
; RUN:     2>&1 | FileCheck %s

; This test checks that VPlan optimizes peeling and peel checks using the
; calculated minimum profitable peeling TC. In particular, we check that:
;   1. If the calculated profitable TC is greater than the expected max TC,
;      VPlan disables the peel (since the check would always be false),
;   2. If the calculated profitable TC is lesser than the expected min TC,
;      VPlan disables the check (since the check would always be true).

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @peel_is_unprofitable(ptr %A, i64 %0) {
; CHECK-LABEL: Selecting VF for VPlan peel_is_unprofitable:HIR.#{{[0-9]+}}
; CHECK:       Selected peeling: Dynamic
; CHECK:       Dynamic peel chosen with unknown (estimated) trip count
; CHECK:       (VF = 4, UF = 1) min profitable peel tc = 120
; CHECK-NEXT:  Calculated min profitable peel TC (120) is greater than the expected max TC (50).
; CHECK-NEXT:  Peel is likely unprofitable: disabling.
; CHECK:       Peeling will not be performed.

; Check no peel loop was emitted
; CHECK:       VPlan IR for: Initial VPlan for VF=4
; CHECK:         [[BB5:BB[0-9]+]]: # preds:
; CHECK-NEXT:     [DA: Uni] i64 [[VP0:%.*]] = add i64 {{.*}} i64 1
; CHECK-NEXT:     [DA: Uni] pushvf VF=4 UF=1
; CHECK-NEXT:     [DA: Uni] i64 [[VP8:%.*]] = vector-trip-count i64 [[VP0]], UF = 1
; CHECK-NEXT:     [DA: Uni] i1 [[VP_VEC_TC_CHECK:%.*]] = icmp eq i64 0 i64 [[VP8]]
; CHECK-NEXT:     [DA: Uni] br i1 [[VP_VEC_TC_CHECK]], [[MERGE_BLK0:merge.blk[0-9]+]], [[BB0:BB[0-9]+]]

entry:
  br label %for.preheader

for.preheader:
  br label %for.body

for.body:                                  ; preds = %for.body, %for.preheader
  %iv = phi i64 [ %0, %for.preheader ], [ %iv.next, %for.body ]
  %A.elem = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %A, i64 %iv)
  store double 0.000000e+00, ptr %A.elem, align 8
  %iv.next = add i64 %iv, 1
  %lftr.wideiv = trunc i64 %iv to i32
  %exitcond = icmp eq i32 0, %lftr.wideiv
  br i1 %exitcond, label %exit, label %for.body, !llvm.loop !0

exit:                       ; preds = %for.body
  br label %for.preheader
}

define void @peel_is_profitable(ptr %A, i64 %0) {
; CHECK-LABEL: Selecting VF for VPlan peel_is_profitable:HIR.#{{[0-9]+}}
; CHECK:       Selected peeling: Dynamic
; CHECK:       Dynamic peel chosen with unknown (estimated) trip count
; CHECK-NEXT:  (VF = 4, UF = 1) min profitable peel tc = 120
; CHECK-NEXT:  Calculated min profitable peel TC (120) is less than the expected min TC (450).
; CHECK-NEXT:  Peel is likely profitable: disabling TC check.
; CHECK:       Peeling will be performed.

; Check peel loop was emitted without a TC check
; CHECK:       VPlan IR for: Initial VPlan for VF=4
; CHECK:         [[PEEL_CHECKZ0:peel.checkz[0-9]+]]: # preds:
; CHECK-NEXT:     [DA: Uni] i64 [[VP0:%.*]] = add i64 {{.*}} i64 1
; CHECK-NEXT:     [DA: Uni] pushvf VF=4 UF=1
; CHECK-NEXT:     [DA: Uni] ptr [[VP_PEEL_BASE_PTR:%.*]] = inv-scev-wrapper
; CHECK-NEXT:     [DA: Uni] i64 [[VP_BASEPTR_INT:%.*]] = ptrtoint ptr [[VP_PEEL_BASE_PTR]] to i64
; CHECK-NEXT:     [DA: Uni] i64 [[VP_QUOTIENT:%.*]] = udiv i64 [[VP_BASEPTR_INT]] i64 8
; CHECK-NEXT:     [DA: Uni] i64 [[VP_QMULTIPLIER:%.*]] = mul i64 [[VP_QUOTIENT]] i64 3
; CHECK-NEXT:     [DA: Uni] i64 [[VP_PEEL_COUNT:%.*]] = urem i64 [[VP_QMULTIPLIER]] i64 4
; CHECK-NEXT:     [DA: Uni] i1 [[VP_PEEL_ZERO_CHECK:%.*]] = icmp eq i64 0 i64 [[VP_PEEL_COUNT]]
; CHECK-NEXT:     [DA: Uni] br i1 [[VP_PEEL_ZERO_CHECK]], [[MERGE_BLK0:merge.blk[0-9]+]], [[PEEL_CHECKV0:peel.checkv[0-9]+]]

entry:
  br label %for.preheader

for.preheader:
  br label %for.body

for.body:                                  ; preds = %for.body, %for.preheader
  %iv = phi i64 [ %0, %for.preheader ], [ %iv.next, %for.body ]
  %A.elem = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %A, i64 %iv)
  store double 0.000000e+00, ptr %A.elem, align 8
  %iv.next = add i64 %iv, 1
  %lftr.wideiv = trunc i64 %iv to i32
  %exitcond = icmp eq i32 0, %lftr.wideiv
  br i1 %exitcond, label %exit, label %for.body, !llvm.loop !4

exit:                       ; preds = %for.body
  br label %for.preheader
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.cos.f64(double) #1

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!0 = distinct !{!0, !1, !2, !3}
!1 = !{!"llvm.loop.intel.loopcount_minimum", i32 10}
!2 = !{!"llvm.loop.intel.loopcount_average", i32 20}
!3 = !{!"llvm.loop.intel.loopcount_maximum", i32 50}
!4 = distinct !{!4, !5, !6, !7}
!5 = !{!"llvm.loop.intel.loopcount_minimum", i32 450}
!6 = !{!"llvm.loop.intel.loopcount_average", i32 500}
!7 = !{!"llvm.loop.intel.loopcount_maximum", i32 600}
