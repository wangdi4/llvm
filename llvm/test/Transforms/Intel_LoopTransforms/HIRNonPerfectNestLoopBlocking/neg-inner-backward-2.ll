; REQUIRES:asserts
; RUN: opt -intel-libirc-allowed --passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output  -debug-only=hir-non-perfect-nest-loop-blocking 2>&1 < %s | FileCheck %s

; Although it is correct to do i2-loop blocking out of i1-loop in this example,
; in general, it is not simple to prove legality with backward dependences on temps.
; We bailout in this case.

; <0>          BEGIN REGION { }
; <45>               + DO i1 = 0, %M + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <46>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <6>                |   |   %2 = (%A)[i2];
; <8>                |   |   %3 = (%B)[i2];
; <10>               |   |   %t.048.us.us = %2 + %3  +  %t.048.us.us;
; <12>               |   |   (%C)[i2] = %t.048.us.us;
; <46>               |   + END LOOP
; <46>               |
; <47>               |
; <47>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <25>               |   |   %0 = (%A)[i2];
; <28>               |   |   %1 = (%B)[i2];
; <31>               |   |   (%D)[i2] = 2 * %0 + %1;
; <47>               |   + END LOOP
; <45>               + END LOOP
; <0>          END REGION
;
; DD graph for function foo:
; 10:10 %t.048.us.us --> %t.048.us.us ANTI (= =) (0 0)
; 8:10 %3 --> %3 FLOW (= =) (0 0)
; 31:31 (%D)[i2] --> (%D)[i2] OUTPUT (* =) (? 0)
; 25:31 %0 --> %0 FLOW (= =) (0 0)
; 10:10 %t.048.us.us --> %t.048.us.us FLOW (<= *) (? ?)
; 10:12 %t.048.us.us --> %t.048.us.us FLOW (= =) (0 0)
; 12:12 (%C)[i2] --> (%C)[i2] OUTPUT (* =) (? 0)
; 6:10 %2 --> %2 FLOW (= =) (0 0)
; 28:31 %1 --> %1 FLOW (= =) (0 0)

; CHECK: Not a candidate
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %M + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:               |   |   %2 = (%A)[i2];
; CHECK:               |   |   %3 = (%B)[i2];
; CHECK:               |   |   %t.048.us.us = %2 + %3  +  %t.048.us.us;
; CHECK:               |   |   (%C)[i2] = %t.048.us.us;
; CHECK:               |   + END LOOP
;                      |
;                      |
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:               |   |   %0 = (%A)[i2];
; CHECK:               |   |   %1 = (%B)[i2];
; CHECK:               |   |   (%D)[i2] = 2 * %0 + %1;
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'test-inner-backward-1.c'
source_filename = "test-inner-backward-1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(i32 noundef %N, i32 noundef %M, ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef readonly %B, ptr noalias nocapture noundef writeonly %C, ptr noalias nocapture noundef writeonly %D) local_unnamed_addr #0 {
entry:
  %cmp47 = icmp sgt i32 %M, 0
  %cmp242 = icmp sgt i32 %N, 0
  %or.cond = and i1 %cmp47, %cmp242
  br i1 %or.cond, label %for.outer.preheader, label %for.cond.cleanup

for.outer.preheader:              ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.outer

for.outer:                        ; preds = %for.outer.preheader, %for.cond11.for.cond.cleanup13_crit_edge.us.us
  %i.049.us.us = phi i32 [ %inc26.us.us, %for.cond11.for.cond.cleanup13_crit_edge.us.us ], [ 0, %for.outer.preheader ]
  %t.048.us.us = phi i32 [ %add7.us.us.lcssa, %for.cond11.for.cond.cleanup13_crit_edge.us.us ], [ 0, %for.outer.preheader ]
  br label %for.body4.us.us

for.body14.us.us:                                 ; preds = %for.cond1.for.cond11.preheader_crit_edge.us.us, %for.body14.us.us
  %indvars.iv72 = phi i64 [ 0, %for.cond1.for.cond11.preheader_crit_edge.us.us ], [ %indvars.iv.next73, %for.body14.us.us ]
  %arrayidx16.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv72
  %0 = load i32, ptr %arrayidx16.us.us, align 4
  %mul.us.us = shl nsw i32 %0, 1
  %arrayidx18.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv72
  %1 = load i32, ptr %arrayidx18.us.us, align 4
  %add19.us.us = add nsw i32 %mul.us.us, %1
  %arrayidx21.us.us = getelementptr inbounds i32, ptr %D, i64 %indvars.iv72
  store i32 %add19.us.us, ptr %arrayidx21.us.us, align 4
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond75.not = icmp eq i64 %indvars.iv.next73, %wide.trip.count
  br i1 %exitcond75.not, label %for.cond11.for.cond.cleanup13_crit_edge.us.us, label %for.body14.us.us

for.body4.us.us:                                  ; preds = %for.body4.us.us, %for.outer
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4.us.us ], [ 0, %for.outer ]
  %t.143.us.us = phi i32 [ %add7.us.us, %for.body4.us.us ], [ %t.048.us.us, %for.outer ]
  %arrayidx.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx.us.us, align 4
  %arrayidx6.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx6.us.us, align 4
  %add.us.us = add nsw i32 %2, %3
  %add7.us.us = add nsw i32 %add.us.us, %t.143.us.us
  %arrayidx9.us.us = getelementptr inbounds i32, ptr %C, i64 %indvars.iv
  store i32 %add7.us.us, ptr %arrayidx9.us.us, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond1.for.cond11.preheader_crit_edge.us.us, label %for.body4.us.us

for.cond1.for.cond11.preheader_crit_edge.us.us:   ; preds = %for.body4.us.us
  %add7.us.us.lcssa = phi i32 [ %add7.us.us, %for.body4.us.us ]
  br label %for.body14.us.us

for.cond11.for.cond.cleanup13_crit_edge.us.us:    ; preds = %for.body14.us.us
  %inc26.us.us = add nuw nsw i32 %i.049.us.us, 1
  %exitcond76.not = icmp eq i32 %inc26.us.us, %M
  br i1 %exitcond76.not, label %for.cond.cleanup.loopexit, label %for.outer

for.cond.cleanup.loopexit:                        ; preds = %for.cond11.for.cond.cleanup13_crit_edge.us.us
  %add7.us.us.lcssa.lcssa = phi i32 [ %add7.us.us.lcssa, %for.cond11.for.cond.cleanup13_crit_edge.us.us ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add7.us.us.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %t.0.lcssa
}
