; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that MAX_TC_EST and LEGAL_MAX_TC are set to 0 in collapsed loop as they overflow int 64.

; HIR before collapse:
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   + DO i2 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   |   + DO i3 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   |   |   + DO i4 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   |   |   |   + DO i5 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   |   |   |   |   (%a)[(zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n))) * i1 + (zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n))) * i2 + (zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n))) * i3 + zext.i16.i64(trunc.i64.i16(%n)) * i4 + i5] = %n;
;                  |   |   |   |   + END LOOP
;                  |   |   |   + END LOOP
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after Collapse 
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, (zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n))) + -1, 1   <DO_LOOP>
; CHECK-NOT: MAX_TC_EST
; CHECK-NOT: LEGAL_MAX_TC
; CHECK:           |   (%a)[i1] = %n;
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %a, i64 noundef %n) local_unnamed_addr {
entry:
  %and = and i64 %n, 65535
  %cmp93.not = icmp eq i64 %and, 0
  br i1 %cmp93.not, label %for.end51, label %for.cond2.preheader.lr.ph

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %conv22 = trunc i64 %n to i16
  %mul = mul nuw nsw i64 %and, %and
  %mul23 = mul nuw nsw i64 %mul, %and
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond2.preheader.lr.ph, %for.cond2.for.inc49_crit_edge
  %indvars.iv105 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %indvars.iv.next106, %for.cond2.for.inc49_crit_edge ]
  %mul26 = mul nuw i64 %indvars.iv105, %mul23
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond2.preheader, %for.cond7.for.inc46_crit_edge
  %indvars.iv102 = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next103, %for.cond7.for.inc46_crit_edge ]
  %mul30 = mul nuw nsw i64 %indvars.iv102, %mul23
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %for.cond7.preheader, %for.cond12.for.inc43_crit_edge
  %indvars.iv99 = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next100, %for.cond12.for.inc43_crit_edge ]
  %mul33 = mul nuw nsw i64 %indvars.iv99, %mul
  %add34 = add nuw i64 %mul33, %mul30
  br label %for.cond17.preheader

for.cond17.preheader:                             ; preds = %for.cond12.preheader, %for.cond17.for.inc40_crit_edge
  %indvars.iv96 = phi i64 [ 0, %for.cond12.preheader ], [ %indvars.iv.next97, %for.cond17.for.inc40_crit_edge ]
  %reass.add = add nuw i64 %indvars.iv96, %mul26
  %reass.mul = mul i64 %reass.add, %and
  %add37 = add i64 %add34, %reass.mul
  br label %for.body21

for.body21:                                       ; preds = %for.cond17.preheader, %for.body21
  %indvars.iv = phi i64 [ 0, %for.cond17.preheader ], [ %indvars.iv.next, %for.body21 ]
  %add39 = add i64 %add37, %indvars.iv
  %arrayidx = getelementptr inbounds i16, ptr %a, i64 %add39
  store i16 %conv22, ptr %arrayidx, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %and
  br i1 %exitcond.not, label %for.cond17.for.inc40_crit_edge, label %for.body21

for.cond17.for.inc40_crit_edge:                   ; preds = %for.body21
  %indvars.iv.next97 = add nuw nsw i64 %indvars.iv96, 1
  %exitcond98.not = icmp eq i64 %indvars.iv.next97, %and
  br i1 %exitcond98.not, label %for.cond12.for.inc43_crit_edge, label %for.cond17.preheader

for.cond12.for.inc43_crit_edge:                   ; preds = %for.cond17.for.inc40_crit_edge
  %indvars.iv.next100 = add nuw nsw i64 %indvars.iv99, 1
  %exitcond101.not = icmp eq i64 %indvars.iv.next100, %and
  br i1 %exitcond101.not, label %for.cond7.for.inc46_crit_edge, label %for.cond12.preheader

for.cond7.for.inc46_crit_edge:                    ; preds = %for.cond12.for.inc43_crit_edge
  %indvars.iv.next103 = add nuw nsw i64 %indvars.iv102, 1
  %exitcond104.not = icmp eq i64 %indvars.iv.next103, %and
  br i1 %exitcond104.not, label %for.cond2.for.inc49_crit_edge, label %for.cond7.preheader

for.cond2.for.inc49_crit_edge:                    ; preds = %for.cond7.for.inc46_crit_edge
  %indvars.iv.next106 = add nuw nsw i64 %indvars.iv105, 1
  %exitcond107.not = icmp eq i64 %indvars.iv.next106, %and
  br i1 %exitcond107.not, label %for.end51.loopexit, label %for.cond2.preheader

for.end51.loopexit:                               ; preds = %for.cond2.for.inc49_crit_edge
  br label %for.end51

for.end51:                                        ; preds = %for.end51.loopexit, %entry
  %div = sdiv i64 %n, 2
  %arrayidx52 = getelementptr inbounds i16, ptr %a, i64 %div
  %0 = load i16, ptr %arrayidx52, align 2
  %conv53 = sext i16 %0 to i32
  ret i32 %conv53
}
