; Check that the remark that was added pre-HIR "survived" HIR phase.

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

; RUN: opt -hir-ssa-deconstruction -intel-loop-optreport=low -hir-post-vec-complete-unroll -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -intel-loop-optreport=low -hir-post-vec-complete-unroll -hir-vec-dir-insert -VPlanDriverHIR -hir-optreport-emitter -hir-cg 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; 1) The tests needs to be rewritten: there are two regions, test checks only the first one
; 2) There may be a bug in optreport code: opt report for the first region wasn't "survived" in the first RUN command
; XFAIL: *

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     Remark: Loop has been unswitched via {{.*}}{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         Remark: Loop has been vectorized with vector {{.*}} factor
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         <Remainder loop for vectorization>
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         Remark: Loop completely unrolled
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture readnone %C, i32* noalias nocapture readnone %D, i32* nocapture %G, i32 %N) local_unnamed_addr #0 {
  %cmp16 = icmp slt i32 0, %N
  br i1 %cmp16, label %.lr.ph19, label %8

.lr.ph19:                                         ; preds = %0
  %cmp113 = icmp slt i32 0, %N
  %G.promoted = load i32, i32* %G, align 4, !tbaa !2
  br i1 %cmp113, label %.lr.ph19.split.us, label %.lr.ph19..lr.ph19.split_crit_edge

.lr.ph19..lr.ph19.split_crit_edge:                ; preds = %.lr.ph19
  br label %.lr.ph19.split

.lr.ph19.split.us:                                ; preds = %.lr.ph19
  br label %1

; <label>:1:                                      ; preds = %7, %.lr.ph19.split.us
  %add21.us = phi i32 [ %G.promoted, %.lr.ph19.split.us ], [ %add.us, %7 ]
  %j.017.us = phi i32 [ 0, %.lr.ph19.split.us ], [ %inc7.us, %7 ]
  br i1 true, label %.lr.ph.us, label %4

.lr.ph.us:                                        ; preds = %1
  br label %2

; <label>:2:                                      ; preds = %2, %.lr.ph.us
  %i.014.us = phi i32 [ 0, %.lr.ph.us ], [ %inc.us, %2 ]
  %3 = zext i32 %i.014.us to i64
  %arrayidx.us = getelementptr inbounds i32, i32* %B, i64 %3
  store i32 %j.017.us, i32* %arrayidx.us, align 4, !tbaa !2
  %inc.us = add nuw nsw i32 %i.014.us, 1
  %cmp1.us = icmp slt i32 %inc.us, %N
  br i1 %cmp1.us, label %2, label %._crit_edge.us

._crit_edge.us:                                   ; preds = %2
  br label %4

; <label>:4:                                      ; preds = %._crit_edge.us, %1
  %add.us = add nsw i32 %add21.us, %j.017.us
  br label %5

; <label>:5:                                      ; preds = %5, %4
  %i2.015.us = phi i32 [ 0, %4 ], [ %inc6.us, %5 ]
  %6 = zext i32 %i2.015.us to i64
  %arrayidx5.us = getelementptr inbounds i32, i32* %A, i64 %6
  store i32 %j.017.us, i32* %arrayidx5.us, align 4, !tbaa !2
  %inc6.us = add nuw nsw i32 %i2.015.us, 1
  %cmp3.us = icmp ult i32 %inc6.us, 3
  br i1 %cmp3.us, label %5, label %7

; <label>:7:                                      ; preds = %5
  %inc7.us = add nuw nsw i32 %j.017.us, 1
  %cmp.us = icmp slt i32 %inc7.us, %N
  br i1 %cmp.us, label %1, label %._crit_edge20.us-lcssa.us, !llvm.loop !6

._crit_edge20.us-lcssa.us:                        ; preds = %7
  %add.lcssa.ph.us = phi i32 [ %add.us, %7 ]
  br label %._crit_edge20

.lr.ph19.split:                                   ; preds = %.lr.ph19..lr.ph19.split_crit_edge
  br label %9

._crit_edge20.us-lcssa:                           ; preds = %13
  %add.lcssa.ph = phi i32 [ %add, %13 ]
  br label %._crit_edge20

._crit_edge20:                                    ; preds = %._crit_edge20.us-lcssa.us, %._crit_edge20.us-lcssa
  %add.lcssa = phi i32 [ %add.lcssa.ph, %._crit_edge20.us-lcssa ], [ %add.lcssa.ph.us, %._crit_edge20.us-lcssa.us ]
  store i32 %add.lcssa, i32* %G, align 4, !tbaa !2
  br label %8

; <label>:8:                                      ; preds = %._crit_edge20, %0
  ret void

; <label>:9:                                      ; preds = %13, %.lr.ph19.split
  %add21 = phi i32 [ %G.promoted, %.lr.ph19.split ], [ %add, %13 ]
  %j.017 = phi i32 [ 0, %.lr.ph19.split ], [ %inc7, %13 ]
  br i1 false, label %.lr.ph, label %10

.lr.ph:                                           ; preds = %9
  br label %11

._crit_edge:                                      ; preds = %11
  br label %10

; <label>:10:                                     ; preds = %._crit_edge, %9
  %add = add nsw i32 %add21, %j.017
  br label %14

; <label>:11:                                     ; preds = %11, %.lr.ph
  %i.014 = phi i32 [ 0, %.lr.ph ], [ %inc, %11 ]
  %12 = zext i32 %i.014 to i64
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %12
  store i32 %j.017, i32* %arrayidx, align 4, !tbaa !2
  %inc = add nuw nsw i32 %i.014, 1
  %cmp1 = icmp slt i32 %inc, %N
  br i1 %cmp1, label %11, label %._crit_edge

; <label>:13:                                     ; preds = %14
  %inc7 = add nuw nsw i32 %j.017, 1
  %cmp = icmp slt i32 %inc7, %N
  br i1 %cmp, label %9, label %._crit_edge20.us-lcssa, !llvm.loop !6

; <label>:14:                                     ; preds = %14, %10
  %i2.015 = phi i32 [ 0, %10 ], [ %inc6, %14 ]
  %15 = zext i32 %i2.015 to i64
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %15
  store i32 %j.017, i32* %arrayidx5, align 4, !tbaa !2
  %inc6 = add nuw nsw i32 %i2.015, 1
  %cmp3 = icmp ult i32 %inc6, 3
  br i1 %cmp3, label %14, label %13
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = distinct !{!"llvm.loop.optreport", !8}
!8 = distinct !{!"intel.loop.optreport", !9}
!9 = !{!"intel.optreport.remarks", !10}
!10 = !{!"intel.optreport.remark", !"Loop has been unswitched via %s", !"cmp113"}
