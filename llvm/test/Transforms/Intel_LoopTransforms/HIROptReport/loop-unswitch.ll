; Check that proper optreport (structure and metadata) is emitted for loop unswitching with HIR passes.

;void foo(int *restrict A, int *restrict B, int *restrict C, int *restrict D, int *G, int N) {
;
;  for (int j = 0; j < N; ++j) {
;    for (int i = 0; i < N; ++i) {
;      B[i] = j;
;    }
;    *G += j;
;    for (int i = 0; i < 3; ++i) {
;      A[i] = j;
;    }
;  }
;  return;
;}

; Check the proper optreport for loop unswitching using metadata.
; RUN: opt -loop-unswitch -intel-loop-optreport=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{[[M1]], [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"llvm.loop.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.loop.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.remarks", [[M5:!.*]]}
; CHECK: [[M5]] = !{!"intel.optreport.remark", !"Loop has been unswitched via %s", {{.*}}}

; Check the proper optreport for loop unswitching.
; RUN: opt -loop-unswitch -intel-loop-optreport=low -intel-ir-optreport-emitter -simplifycfg < %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-EMITTER --strict-whitespace

; CHECK-EMITTER: LOOP BEGIN
; CHECK-EMITTER-NEXT:     Remark: Loop has been unswitched via cmp113{{[[:space:]]}}
; CHECK-EMITTER-NEXT:     LOOP BEGIN
; CHECK-EMITTER-NEXT:     LOOP END{{[[:space:]]}}
; CHECK-EMITTER-NEXT:     LOOP BEGIN
; CHECK-EMITTER-NEXT:     LOOP END
; CHECK-EMITTER-NEXT: LOOP END

; RUN: opt -loop-unswitch -intel-loop-optreport=low -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=CHECK-HIR --strict-whitespace

; CHECK-HIR: LOOP BEGIN
; CHECK-HIR-NEXT:     Remark: Loop has been unswitched via {{.*}}{{[[:space:]]}}
; CHECK-HIR-NEXT:     LOOP BEGIN
; CHECK-HIR-NEXT:         Remark: Loop has been vectorized with vector {{.*}} factor
; CHECK-HIR-NEXT:     LOOP END{{[[:space:]]}}
; CHECK-HIR-NEXT:     LOOP BEGIN
; CHECK-HIR-NEXT:         <Remainder loop for vectorization>
; CHECK-HIR-NEXT:     LOOP END{{[[:space:]]}}
; CHECK-HIR-NEXT:     LOOP BEGIN
; CHECK-HIR-NEXT:         Remark: Loop completely unrolled
; CHECK-HIR-NEXT:     LOOP END
; CHECK-HIR-NEXT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture readnone %C, i32* noalias nocapture readnone %D, i32* nocapture %G, i32 %N) local_unnamed_addr #0 {
  %cmp16 = icmp slt i32 0, %N
  br i1 %cmp16, label %.lr.ph19, label %1

.lr.ph19:                                         ; preds = %0
  %cmp113 = icmp slt i32 0, %N
  %G.promoted = load i32, i32* %G, align 4, !tbaa !2
  br label %2

._crit_edge20:                                    ; preds = %6
  %add.lcssa = phi i32 [ %add, %6 ]
  store i32 %add.lcssa, i32* %G, align 4, !tbaa !2
  br label %1

; <label>:1:                                      ; preds = %._crit_edge20, %0
  ret void

; <label>:2:                                      ; preds = %.lr.ph19, %6
  %add21 = phi i32 [ %G.promoted, %.lr.ph19 ], [ %add, %6 ]
  %j.017 = phi i32 [ 0, %.lr.ph19 ], [ %inc7, %6 ]
  br i1 %cmp113, label %.lr.ph, label %3

.lr.ph:                                           ; preds = %2
  br label %4

._crit_edge:                                      ; preds = %4
  br label %3

; <label>:3:                                      ; preds = %._crit_edge, %2
  %add = add nsw i32 %add21, %j.017
  br label %7

; <label>:4:                                      ; preds = %.lr.ph, %4
  %i.014 = phi i32 [ 0, %.lr.ph ], [ %inc, %4 ]
  %5 = zext i32 %i.014 to i64
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %5
  store i32 %j.017, i32* %arrayidx, align 4, !tbaa !2
  %inc = add nuw nsw i32 %i.014, 1
  %cmp1 = icmp slt i32 %inc, %N
  br i1 %cmp1, label %4, label %._crit_edge

; <label>:6:                                      ; preds = %7
  %inc7 = add nuw nsw i32 %j.017, 1
  %cmp = icmp slt i32 %inc7, %N
  br i1 %cmp, label %2, label %._crit_edge20

; <label>:7:                                      ; preds = %3, %7
  %i2.015 = phi i32 [ 0, %3 ], [ %inc6, %7 ]
  %8 = zext i32 %i2.015 to i64
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %8
  store i32 %j.017, i32* %arrayidx5, align 4, !tbaa !2
  %inc6 = add nuw nsw i32 %i2.015, 1
  %cmp3 = icmp ult i32 %inc6, 3
  br i1 %cmp3, label %7, label %6
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
