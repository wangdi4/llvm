; RUN: opt -intel-libirc-allowed -hir-non-perfect-nest-loop-blocking-stripmine-size=2 --passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output 2>&1 < %s | FileCheck %s

; Verify that HIRNonPerfectNestLoopBlocking is done.
; Backward depence between A[i2] doesn't matter.


; CHECK: Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:              |   |   %2 = (%A)[i2];
; CHECK:              |   |   %3 = (%B)[i2];
; CHECK:              |   |   (%C)[i2] = %2 + %3;
; CHECK:              |   + END LOOP
;                     |
;                     |
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>
; CHECK:              |   |   %0 = (%A)[i2];
; CHECK:              |   |   %1 = (%B)[i2];
; CHECK:              |   |   (%A)[i2] = 2 * %0 + %1;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; DD graph for function foo:
; Region 0:
; 6:11 %2 --> %2 FLOW (= =) (0 0)
; 6:28 (%A)[i2] --> (%A)[i2] ANTI (*) (?)
; 26:28 %1 --> %1 FLOW (= =) (0 0)
; 23:28 %0 --> %0 FLOW (= =) (0 0)
; 11:11 (%C)[i2] --> (%C)[i2] OUTPUT (* =) (? 0)
; 23:28 (%A)[i2] --> (%A)[i2] ANTI (* =) (? 0)
; 28:6 (%A)[i2] --> (%A)[i2] FLOW (*) (?)      // no problem.
; 28:23 (%A)[i2] --> (%A)[i2] FLOW (* =) (? 0) // no problem.
; 28:28 (%A)[i2] --> (%A)[i2] OUTPUT (* =) (? 0)
; 8:11 %3 --> %3 FLOW (= =) (0 0)

; CHECK:  BEGIN REGION { modified }
; CHECK:   + DO i1 = 0, (-1 + sext.i32.i64(%N)), 2   <DO_LOOP>
; CHECK:   |   %tile_e_min = (i1 + 1 <= (-1 + sext.i32.i64(%N))) ? i1 + 1 : (-1 + sext.i32.i64(%N));
;          |
; CHECK:   |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:   |   |   %lb_max = (0 <= i1) ? i1 : 0;
; CHECK:   |   |   %ub_min = (sext.i32.i64(%N) + -1 <= %tile_e_min) ? sext.i32.i64(%N) + -1 : %tile_e_min;
;          |   |
; CHECK:   |   |   + DO i3 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; CHECK:   |   |   |   %2 = (%A)[i3 + %lb_max];
; CHECK:   |   |   |   %3 = (%B)[i3 + %lb_max];
; CHECK:   |   |   |   (%C)[i3 + %lb_max] = %2 + %3;
; CHECK:   |   |   + END LOOP
;          |   |
; CHECK:   |   |   %lb_max[[V3:.*]] = (0 <= i1) ? i1 : 0;
; CHECK:   |   |   %ub_min[[V4:.*]] = (sext.i32.i64(%N) + -1 <= %tile_e_min) ? sext.i32.i64(%N) + -1 : %tile_e_min;
;          |   |
; CHECK:   |   |   + DO i3 = 0, -1 * %lb_max[[V3]] + %ub_min[[V4]], 1   <DO_LOOP>
; CHECK:   |   |   |   %0 = (%A)[i3 + %lb_max[[V3]]];
; CHECK:   |   |   |   %1 = (%B)[i3 + %lb_max[[V3]]];
; CHECK:   |   |   |   (%A)[i3 + %lb_max[[V3]]] = 2 * %0 + %1;
; CHECK:   |   |   + END LOOP
; CHECK:   |   + END LOOP
; CHECK:   + END LOOP
; CHECK:  END REGION

;Module Before HIR
; ModuleID = 'test-backward-memref.c'
source_filename = "test-backward-memref.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(i32 noundef %N, i32 noundef %M, ptr noalias nocapture noundef %A, ptr noalias nocapture noundef readonly %B, ptr noalias nocapture noundef writeonly %C, ptr noalias nocapture noundef readnone %D) local_unnamed_addr #0 {
entry:
  %cmp44 = icmp sgt i32 %M, 0
  %cmp1142 = icmp sgt i32 %N, 0
  %or.cond = and i1 %cmp44, %cmp1142
  br i1 %or.cond, label %for.outer.preheader, label %for.cond.cleanup

for.outer.preheader:              ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.outer

for.outer:                        ; preds = %for.outer.preheader, %for.cond10.for.cond.cleanup12_crit_edge.us.us
  %i.045.us.us = phi i32 [ %inc25.us.us, %for.cond10.for.cond.cleanup12_crit_edge.us.us ], [ 0, %for.outer.preheader ]
  br label %for.body4.us.us

for.body13.us.us:                                 ; preds = %for.body13.us.us.preheader, %for.body13.us.us
  %indvars.iv63 = phi i64 [ %indvars.iv.next64, %for.body13.us.us ], [ 0, %for.body13.us.us.preheader ]
  %arrayidx15.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv63
  %0 = load i32, ptr %arrayidx15.us.us, align 4
  %mul.us.us = shl nsw i32 %0, 1
  %arrayidx17.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv63
  %1 = load i32, ptr %arrayidx17.us.us, align 4
  %add18.us.us = add nsw i32 %mul.us.us, %1
  store i32 %add18.us.us, ptr %arrayidx15.us.us, align 4
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond66.not = icmp eq i64 %indvars.iv.next64, %wide.trip.count
  br i1 %exitcond66.not, label %for.cond10.for.cond.cleanup12_crit_edge.us.us, label %for.body13.us.us

for.body4.us.us:                                  ; preds = %for.outer, %for.body4.us.us
  %indvars.iv = phi i64 [ 0, %for.outer ], [ %indvars.iv.next, %for.body4.us.us ]
  %arrayidx.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx.us.us, align 4
  %arrayidx6.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx6.us.us, align 4
  %add.us.us = add nsw i32 %3, %2
  %arrayidx8.us.us = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  store i32 %add.us.us, ptr %arrayidx8.us.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.body13.us.us.preheader, label %for.body4.us.us

for.body13.us.us.preheader:                       ; preds = %for.body4.us.us
  br label %for.body13.us.us

for.cond10.for.cond.cleanup12_crit_edge.us.us:    ; preds = %for.body13.us.us
  %inc25.us.us = add nuw nsw i32 %i.045.us.us, 1
  %exitcond67.not = icmp eq i32 %inc25.us.us, %M
  br i1 %exitcond67.not, label %for.cond.cleanup.loopexit, label %for.outer

for.cond.cleanup.loopexit:                        ; preds = %for.cond10.for.cond.cleanup12_crit_edge.us.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 0
}
