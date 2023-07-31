; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that two "DO i2" loops will not be fused because of:
; (%a)[0][i2][i3] -> (%a)[0][3][i3] flow edge

; BEGIN REGION { }
;       + DO i1 = 0, 9999, 1   <DO_LOOP>
;       |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;       |   |   + DO i3 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;       |   |   |   (%a)[0][i2][i3] = i1 + i2;
;       |   |   + END LOOP
;       |   + END LOOP
;       |
;       |
;       |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
;       |   |   |   %0 = (%a)[0][3][i3];
;       |   |   |   %1 = (%b)[i3];
;       |   |   |   (%b)[i3] = %0 + %1;
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK: DO i1

; CHECK:   DO i2
; CHECK:     DO i3
; CHECK:     END LOOP
; CHECK:   END LOOP

; CHECK:   DO i2
; CHECK:     DO i3
; CHECK:     END LOOP
; CHECK:   END LOOP

; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %a, i32 %n, ptr noalias nocapture %b) local_unnamed_addr #0 {
entry:
  %conv = sext i32 %n to i64
  %cmp268 = icmp sgt i32 %n, 0
  br i1 %cmp268, label %for.cond6.preheader.us.us.preheader.preheader, label %for.cond.cleanup23.preheader

for.cond.cleanup23.preheader:                     ; preds = %entry
  br label %for.cond.cleanup23

for.cond6.preheader.us.us.preheader.preheader:    ; preds = %entry
  br label %for.cond6.preheader.us.us.preheader

for.cond6.preheader.us.us.preheader:              ; preds = %for.cond6.preheader.us.us.preheader.preheader, %for.cond.cleanup23.us
  %i.075.us = phi i64 [ %inc44.us, %for.cond.cleanup23.us ], [ 0, %for.cond6.preheader.us.us.preheader.preheader ]
  br label %for.cond6.preheader.us.us

for.cond.cleanup23.us:                            ; preds = %for.cond26.for.cond.cleanup30_crit_edge.us.us
  %inc44.us = add nuw nsw i64 %i.075.us, 1
  %exitcond117 = icmp eq i64 %inc44.us, 10000
  br i1 %exitcond117, label %for.cond.cleanup.loopexit, label %for.cond6.preheader.us.us.preheader

for.cond6.preheader.us.us:                        ; preds = %for.cond6.for.cond.cleanup10_crit_edge.us.us, %for.cond6.preheader.us.us.preheader
  %j.069.us.us = phi i64 [ %inc16.us.us, %for.cond6.for.cond.cleanup10_crit_edge.us.us ], [ 0, %for.cond6.preheader.us.us.preheader ]
  %add.us.us = add nuw nsw i64 %j.069.us.us, %i.075.us
  %conv12.us.us = trunc i64 %add.us.us to i32
  br label %for.body11.us.us

for.cond6.for.cond.cleanup10_crit_edge.us.us:     ; preds = %for.body11.us.us
  %inc16.us.us = add nuw nsw i64 %j.069.us.us, 1
  %exitcond114 = icmp eq i64 %inc16.us.us, %conv
  br i1 %exitcond114, label %for.cond26.preheader.us.us.preheader, label %for.cond6.preheader.us.us

for.cond26.preheader.us.us.preheader:             ; preds = %for.cond6.for.cond.cleanup10_crit_edge.us.us
  br label %for.cond26.preheader.us.us

for.body11.us.us:                                 ; preds = %for.body11.us.us, %for.cond6.preheader.us.us
  %k.067.us.us = phi i64 [ 0, %for.cond6.preheader.us.us ], [ %inc.us.us, %for.body11.us.us ]
  %arrayidx14.us.us = getelementptr inbounds [10 x [10 x i32]], ptr %a, i64 0, i64 %j.069.us.us, i64 %k.067.us.us
  store i32 %conv12.us.us, ptr %arrayidx14.us.us, align 4
  %inc.us.us = add nuw nsw i64 %k.067.us.us, 1
  %exitcond = icmp eq i64 %inc.us.us, %conv
  br i1 %exitcond, label %for.cond6.for.cond.cleanup10_crit_edge.us.us, label %for.body11.us.us

for.cond26.preheader.us.us:                       ; preds = %for.cond26.preheader.us.us.preheader, %for.cond26.for.cond.cleanup30_crit_edge.us.us
  %j18.074.us.us = phi i64 [ %inc41.us.us, %for.cond26.for.cond.cleanup30_crit_edge.us.us ], [ 0, %for.cond26.preheader.us.us.preheader ]
  br label %for.body31.us.us

for.cond26.for.cond.cleanup30_crit_edge.us.us:    ; preds = %for.body31.us.us
  %inc41.us.us = add nuw nsw i64 %j18.074.us.us, 1
  %exitcond116 = icmp eq i64 %inc41.us.us, %conv
  br i1 %exitcond116, label %for.cond.cleanup23.us, label %for.cond26.preheader.us.us

for.body31.us.us:                                 ; preds = %for.body31.us.us, %for.cond26.preheader.us.us
  %k25.072.us.us = phi i64 [ 0, %for.cond26.preheader.us.us ], [ %inc38.us.us, %for.body31.us.us ]
  %arrayidx34.us.us = getelementptr inbounds [10 x [10 x i32]], ptr %a, i64 0, i64 3, i64 %k25.072.us.us
  %0 = load i32, ptr %arrayidx34.us.us, align 4
  %arrayidx35.us.us = getelementptr inbounds i32, ptr %b, i64 %k25.072.us.us
  %1 = load i32, ptr %arrayidx35.us.us, align 4
  %add36.us.us = add nsw i32 %1, %0
  store i32 %add36.us.us, ptr %arrayidx35.us.us, align 4
  %inc38.us.us = add nuw nsw i64 %k25.072.us.us, 1
  %exitcond115 = icmp eq i64 %inc38.us.us, %conv
  br i1 %exitcond115, label %for.cond26.for.cond.cleanup30_crit_edge.us.us, label %for.body31.us.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup23.us
  br label %for.cond.cleanup

for.cond.cleanup.loopexit122:                     ; preds = %for.cond.cleanup23
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit122, %for.cond.cleanup.loopexit
  ret void

for.cond.cleanup23:                               ; preds = %for.cond.cleanup23.preheader, %for.cond.cleanup23
  %i.075 = phi i64 [ %inc44, %for.cond.cleanup23 ], [ 0, %for.cond.cleanup23.preheader ]
  %inc44 = add nuw nsw i64 %i.075, 1
  %exitcond120 = icmp eq i64 %inc44, 10000
  br i1 %exitcond120, label %for.cond.cleanup.loopexit122, label %for.cond.cleanup23
}

