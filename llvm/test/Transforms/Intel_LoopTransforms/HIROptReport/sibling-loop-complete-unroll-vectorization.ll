; Check the loop opt reports are printed with vectorization and complete unroll along with simplify CFG in presence of sibling loops.

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

; TODO: There is still a small issue where the merged CFG-based HIR causes
; the opt report layout to mismatch slightly with the loop layout.
; RUN: opt -intel-opt-report=low -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -vplan-force-vf=4 2>&1 < %s -S | FileCheck %s -check-prefix=MERGED-CFG-HIR --strict-whitespace

; MERGED-CFG-HIR:      LOOP BEGIN
; MERGED-CFG-HIR:          LOOP BEGIN
; MERGED-CFG-HIR-NEXT:         remark #15300: LOOP WAS VECTORIZED
; MERGED-CFG-HIR-NEXT:         remark #15305: vectorization support: vector length {{.*}}
; MERGED-CFG-HIR-NEXT:     LOOP END{{[[:space:]]}}
; MERGED-CFG-HIR-NEXT:     LOOP BEGIN
; MERGED-CFG-HIR-NEXT:         remark #25436: Loop completely unrolled by 3
; MERGED-CFG-HIR-NEXT:     LOOP END{{[[:space:]]}}
; MERGED-CFG-HIR-NEXT:     LOOP BEGIN
; MERGED-CFG-HIR-NEXT:         <Remainder loop for vectorization>
; MERGED-CFG-HIR-NEXT:     LOOP END
; MERGED-CFG-HIR-NEXT: LOOP END
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture %A, ptr noalias nocapture %B, ptr noalias nocapture readnone %C, ptr noalias nocapture readnone %D, ptr nocapture %G, i32 %N) local_unnamed_addr #0 {
  %cmp16 = icmp slt i32 0, %N
  br i1 %cmp16, label %.lr.ph19, label %1

.lr.ph19:                                         ; preds = %0
  %cmp113 = icmp slt i32 0, %N
  %G.promoted = load i32, ptr %G, align 4, !tbaa !2
  br label %2

._crit_edge20:                                    ; preds = %6
  %add.lcssa = phi i32 [ %add, %6 ]
  store i32 %add.lcssa, ptr %G, align 4, !tbaa !2
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
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %5
  store i32 %j.017, ptr %arrayidx, align 4, !tbaa !2
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
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 %8
  store i32 %j.017, ptr %arrayidx5, align 4, !tbaa !2
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
