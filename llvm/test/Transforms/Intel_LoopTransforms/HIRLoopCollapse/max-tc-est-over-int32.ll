; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that MAX_TC_EST is set to 0 in collapsed loop as it is over unsigned max int 32.

; HIR before collapse:
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   + DO i2 = 0, zext.i16.i64(trunc.i64.i16(%n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>  <LEGAL_MAX_TC = 65535>
;                  |   |   + DO i3 = 0, 4, 1   <DO_LOOP>
;                  |   |   |   (%a)[5 * zext.i16.i64(trunc.i64.i16(%n)) * i1 + 5 * i2 + i3] = %n;
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR After collapse:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 5 * (zext.i16.i64(trunc.i64.i16(%n)) * zext.i16.i64(trunc.i64.i16(%n))) + -1, 1   <DO_LOOP>
; CHECK-NOT:MAX_TC_EST
; CHECK-NOT:LEGAL_MAX+TC
; CHECK:           |   (%a)[i1] = %n;
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %a, i64 noundef %n) local_unnamed_addr {
entry:
  %and = and i64 %n, 65535
  %cmp40.not = icmp eq i64 %and, 0
  br i1 %cmp40.not, label %for.end23, label %for.cond2.preheader.lr.ph

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %conv11 = trunc i64 %n to i16
  %mul = mul nuw nsw i64 %and, 5
  br label %for.cond7.preheader.lr.ph

for.cond7.preheader.lr.ph:                        ; preds = %for.inc21, %for.cond2.preheader.lr.ph
  %indvars.iv47 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %indvars.iv.next48, %for.inc21 ]
  %mul13 = mul nuw nsw i64 %mul, %indvars.iv47
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond7.preheader.lr.ph, %for.inc18
  %indvars.iv43 = phi i64 [ 0, %for.cond7.preheader.lr.ph ], [ %indvars.iv.next44, %for.inc18 ]
  %0 = mul nuw nsw i64 %indvars.iv43, 5
  %add = add nuw nsw i64 %mul13, %0
  br label %for.body10

for.body10:                                       ; preds = %for.cond7.preheader, %for.body10
  %indvars.iv = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next, %for.body10 ]
  %add17 = add nuw nsw i64 %add, %indvars.iv
  %arrayidx = getelementptr inbounds i16, ptr %a, i64 %add17
  store i16 %conv11, ptr %arrayidx, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond.not, label %for.inc18, label %for.body10

for.inc18:                                        ; preds = %for.body10
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond46.not = icmp eq i64 %indvars.iv.next44, %and
  br i1 %exitcond46.not, label %for.inc21, label %for.cond7.preheader

for.inc21:                                        ; preds = %for.inc18
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond49.not = icmp eq i64 %indvars.iv.next48, %and
  br i1 %exitcond49.not, label %for.end23.loopexit, label %for.cond7.preheader.lr.ph

for.end23.loopexit:                               ; preds = %for.inc21
  br label %for.end23

for.end23:                                        ; preds = %for.end23.loopexit, %entry
  %div = sdiv i64 %n, 2
  %arrayidx24 = getelementptr inbounds i16, ptr %a, i64 %div
  %1 = load i16, ptr %arrayidx24, align 2
  %conv25 = sext i16 %1 to i32
  ret i32 %conv25
}

