; REQUIRES:asserts
; RUN: opt -intel-libirc-allowed --passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output  -debug-only=hir-non-perfect-nest-loop-blocking 2>&1 < %s | FileCheck %s

; Although it is correct to do i2-loop blocking out of i1-loop in this example,
; in general, it is not simple to prove legaltiy with backward dependences on temps.
; We bailout in this case.

; Function: foo
;
; <0>          BEGIN REGION { }
; <43>               + DO i1 = 0, %M + -1, 1
; <44>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; <6>                |   |   %2 = (%A)[i2];
; <8>                |   |   %3 = (%B)[i2];
; <10>               |   |   %t.044.us.us = %2 + %3  +  %t.044.us.us;
; <44>               |   + END LOOP
; <44>               |
; <45>               |
; <45>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; <23>               |   |   %0 = (%A)[i2];
; <26>               |   |   %1 = (%B)[i2];
; <29>               |   |   (%C)[i2] = 2 * %0 + %1;
; <45>               |   + END LOOP
; <43>               + END LOOP
; <0>          END REGION

; DD graph for function foo:
; 8:10 %3 --> %3 FLOW (= =) (0 0)
; 10:10 %t.044.us.us --> %t.044.us.us ANTI (= =) (0 0)
; 6:10 %2 --> %2 FLOW (= =) (0 0)
; 29:29 (%C)[i2] --> (%C)[i2] OUTPUT (* =) (? 0)
; 26:29 %1 --> %1 FLOW (= =) (0 0)
; 23:29 %0 --> %0 FLOW (= =) (0 0)
; 10:10 %t.044.us.us --> %t.044.us.us FLOW (<= *) (? ?)

; CHECK: Not a candidate
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %M + -1, 1
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; CHECK:               |   |   %2 = (%A)[i2];
; CHECK:               |   |   %3 = (%B)[i2];
; CHECK:               |   |   %t.044.us.us = %2 + %3  +  %t.044.us.us;
; CHECK:               |   + END LOOP
;                      |
;                      |
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; CHECK:               |   |   %0 = (%A)[i2];
; CHECK:               |   |   %1 = (%B)[i2];
; CHECK:               |   |   (%C)[i2] = 2 * %0 + %1;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


; ModuleID = 'test-inner-backward.c'
source_filename = "test-inner-backward.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(i32 noundef %N, i32 noundef %M, ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef readonly %B, ptr noalias nocapture noundef writeonly %C) local_unnamed_addr #0 {
entry:
  %cmp43 = icmp sgt i32 %M, 0
  %cmp1041 = icmp sgt i32 %N, 0
  %or.cond = and i1 %cmp43, %cmp1041
  br i1 %or.cond, label %for.outer.preheader, label %for.cond.cleanup

for.outer.preheader:              ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.outer

for.outer:                        ; preds = %for.outer.preheader, %for.outer.latch
  %i.045.us.us = phi i32 [ %inc24.us.us, %for.outer.latch ], [ 0, %for.outer.preheader ]
  %t.044.us.us = phi i32 [ %add7.us.us.lcssa, %for.outer.latch ], [ 0, %for.outer.preheader ]
  br label %for.body4.us.us

for.body12.us.us:                                 ; preds = %for.cond1.for.cond9.preheader_crit_edge.us.us, %for.body12.us.us
  %indvars.iv71 = phi i64 [ 0, %for.cond1.for.cond9.preheader_crit_edge.us.us ], [ %indvars.iv.next72, %for.body12.us.us ]
  %arrayidx14.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv71
  %0 = load i32, ptr %arrayidx14.us.us, align 4
  %mul.us.us = shl nsw i32 %0, 1
  %arrayidx16.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv71
  %1 = load i32, ptr %arrayidx16.us.us, align 4
  %add17.us.us = add nsw i32 %mul.us.us, %1
  %arrayidx19.us.us = getelementptr inbounds i32, ptr %C, i64 %indvars.iv71
  store i32 %add17.us.us, ptr %arrayidx19.us.us, align 4
  %indvars.iv.next72 = add nuw nsw i64 %indvars.iv71, 1
  %exitcond74.not = icmp eq i64 %indvars.iv.next72, %wide.trip.count
  br i1 %exitcond74.not, label %for.outer.latch, label %for.body12.us.us

for.body4.us.us:                                  ; preds = %for.outer, %for.body4.us.us
  %indvars.iv = phi i64 [ 0, %for.outer ], [ %indvars.iv.next, %for.body4.us.us ]
  %t.139.us.us = phi i32 [ %t.044.us.us, %for.outer ], [ %add7.us.us, %for.body4.us.us ]
  %arrayidx.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx.us.us, align 4
  %arrayidx6.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx6.us.us, align 4
  %add.us.us = add nsw i32 %2, %3
  %add7.us.us = add nsw i32 %add.us.us, %t.139.us.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond1.for.cond9.preheader_crit_edge.us.us, label %for.body4.us.us

for.cond1.for.cond9.preheader_crit_edge.us.us:    ; preds = %for.body4.us.us
  %add7.us.us.lcssa = phi i32 [ %add7.us.us, %for.body4.us.us ]
  br label %for.body12.us.us

for.outer.latch:     ; preds = %for.body12.us.us
  %inc24.us.us = add nuw nsw i32 %i.045.us.us, 1
  %exitcond75.not = icmp eq i32 %inc24.us.us, %M
  br i1 %exitcond75.not, label %for.cond.cleanup.loopexit, label %for.outer

for.cond.cleanup.loopexit:                        ; preds = %for.outer.latch
  %add7.us.us.lcssa.lcssa = phi i32 [ %add7.us.us.lcssa, %for.outer.latch ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add7.us.us.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %t.0.lcssa
}
