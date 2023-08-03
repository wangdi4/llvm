; REQUIRES:asserts
; RUN: opt -intel-libirc-allowed --passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-perfect-nest-loop-blocking,print<hir>" -disable-hir-non-perfect-nest-loop-blocking=false -disable-output  -debug-only=hir-non-perfect-nest-loop-blocking 2>&1 < %s | FileCheck %s


; Bail out over <10> to <23>' Flow edge via %t.036.us.us
; That is covered by the existence of liveout of for-loop at 9.
; Notice that backward flow edge 10:10 is also currently preventing loop blocking of i2,
; but without the use at <23>, it could have been legal to block i2-loop out of i1-loop.

; Function: foo
;
; <0>          BEGIN REGION { }
; <37>               + DO i1 = 0, %M + -1, 1
; <38>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; <6>                |   |   %0 = (%A)[i2];
; <8>                |   |   %1 = (%B)[i2];
; <10>               |   |   %t.036.us.us = %t.036.us.us  +  %0 + %1;
; <38>               |   + END LOOP
; <38>               |
; <39>               |
; <39>               |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1
; <23>               |   |   (%C)[i2] = %t.036.us.us;
; <39>               |   + END LOOP
; <37>               + END LOOP
; <0>          END REGION
;
; DD graph for function foo:
; 10:10 %t.036.us.us --> %t.036.us.us ANTI (= =) (0 0)
; 8:10 %1 --> %1 FLOW (= =) (0 0)
; 10:10 %t.036.us.us --> %t.036.us.us FLOW (<= *) (? ?) // Issue - backward
; 10:23 %t.036.us.us --> %t.036.us.us FLOW (=) (0) // Issue - forward
; 6:10 %0 --> %0 FLOW (= =) (0 0)

; CHECK: Inner loops have liveouts.
; CHECK: No candidate was found in this region.
; Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, %M + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %0 = (%A)[i2];
; CHECK:              |   |   %1 = (%B)[i2];
; CHECK:              |   |   %t.036.us.us = %t.036.us.us  +  %0 + %1;
; CHECK:              |   + END LOOP
;                     |
;                     |
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   (%C)[i2] = %t.036.us.us;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION


; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @foo(i32 noundef %N, i32 noundef %M, ptr noalias nocapture noundef readonly %A, ptr noalias nocapture noundef %B, ptr noalias nocapture noundef %C) local_unnamed_addr #0 {
entry:
  %cmp35 = icmp sgt i32 %M, 0
  %cmp230 = icmp sgt i32 %N, 0
  %or.cond = and i1 %cmp35, %cmp230
  br i1 %or.cond, label %for.cond1.preheader.us.us.preheader, label %for.cond.cleanup

for.cond1.preheader.us.us.preheader:              ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.cond1.preheader.us.us

for.cond1.preheader.us.us:                        ; preds = %for.cond1.preheader.us.us.preheader, %for.cond9.for.cond.cleanup11_crit_edge.us.us
  %i.037.us.us = phi i32 [ %inc19.us.us, %for.cond9.for.cond.cleanup11_crit_edge.us.us ], [ 0, %for.cond1.preheader.us.us.preheader ]
  %t.036.us.us = phi i32 [ %add7.us.us.lcssa, %for.cond9.for.cond.cleanup11_crit_edge.us.us ], [ 0, %for.cond1.preheader.us.us.preheader ]
  br label %for.body4.us.us

for.body12.us.us:                                 ; preds = %for.cond1.for.cond9.preheader_crit_edge.us.us, %for.body12.us.us
  %indvars.iv52 = phi i64 [ 0, %for.cond1.for.cond9.preheader_crit_edge.us.us ], [ %indvars.iv.next53, %for.body12.us.us ]
  %arrayidx14.us.us = getelementptr inbounds i32, ptr %C, i64 %indvars.iv52
  store i32 %add7.us.us.lcssa, ptr %arrayidx14.us.us, align 4
  %indvars.iv.next53 = add nuw nsw i64 %indvars.iv52, 1
  %exitcond55.not = icmp eq i64 %indvars.iv.next53, %wide.trip.count
  br i1 %exitcond55.not, label %for.cond9.for.cond.cleanup11_crit_edge.us.us, label %for.body12.us.us

for.body4.us.us:                                  ; preds = %for.body4.us.us, %for.cond1.preheader.us.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4.us.us ], [ 0, %for.cond1.preheader.us.us ]
  %t.131.us.us = phi i32 [ %add7.us.us, %for.body4.us.us ], [ %t.036.us.us, %for.cond1.preheader.us.us ]
  %arrayidx.us.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx.us.us, align 4
  %arrayidx6.us.us = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6.us.us, align 4
  %add.us.us = add nsw i32 %0, %1
  %add7.us.us = add nsw i32 %t.131.us.us, %add.us.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond1.for.cond9.preheader_crit_edge.us.us, label %for.body4.us.us

for.cond1.for.cond9.preheader_crit_edge.us.us:    ; preds = %for.body4.us.us
  %add7.us.us.lcssa = phi i32 [ %add7.us.us, %for.body4.us.us ]
  br label %for.body12.us.us

for.cond9.for.cond.cleanup11_crit_edge.us.us:     ; preds = %for.body12.us.us
  %inc19.us.us = add nuw nsw i32 %i.037.us.us, 1
  %exitcond56.not = icmp eq i32 %inc19.us.us, %M
  br i1 %exitcond56.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader.us.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond9.for.cond.cleanup11_crit_edge.us.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}
