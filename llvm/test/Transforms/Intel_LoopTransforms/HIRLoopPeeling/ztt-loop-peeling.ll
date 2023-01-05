; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-peeling" -disable-output -print-before=hir-loop-peeling -print-after=hir-loop-peeling -hir-details 2>&1 < %s | FileCheck %s

; Verify peeling works correctly in the presence of ZTT.
; We extract the ZTT, preheader and postexit and then peel first iteration.
; Peeled loop gets a new ZTT.

; CHECK: Dump Before

; CHECK: + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |      %t.116 = %t.019;
; CHECK: |   + DO i64 i2 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   (%A)[i2] = %t.116;
; CHECK: |   |   %t.116 = i2;
; CHECK: |   + END LOOP
; CHECK: |      %t.019 = %m + -1;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   if (%m > 0)
; CHECK: |   {
; CHECK: |      %t.116 = %t.019;
; CHECK: |      (%A)[0] = %t.116;
; CHECK: |
; CHECK: |      + Ztt: if (0 < zext.i32.i64(%m) + -1)
; CHECK: |      + DO i64 i2 = 0, zext.i32.i64(%m) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |      |   (%A)[i2 + 1] = i2;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %t.019 = %m + -1;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture noundef writeonly %A, i32 noundef %n, i32 noundef %m) {
entry:
  %cmp17 = icmp sgt i32 %n, 0
  br i1 %cmp17, label %for.cond1.preheader.lr.ph, label %for.end6

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp214 = icmp sgt i32 %m, 0
  %0 = add i32 %m, -1
  %wide.trip.count = zext i32 %m to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc4
  %t.019 = phi i32 [ 5, %for.cond1.preheader.lr.ph ], [ %t.1.lcssa, %for.inc4 ]
  %i.018 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc5, %for.inc4 ]
  br i1 %cmp214, label %for.body3.preheader, label %for.inc4

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %t.116 = phi i32 [ %1, %for.body3 ], [ %t.019, %for.body3.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %t.116, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %1 = trunc i64 %indvars.iv to i32
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc4.loopexit, label %for.body3

for.inc4.loopexit:                                ; preds = %for.body3
  br label %for.inc4

for.inc4:                                         ; preds = %for.inc4.loopexit, %for.cond1.preheader
  %t.1.lcssa = phi i32 [ %t.019, %for.cond1.preheader ], [ %0, %for.inc4.loopexit ]
  %inc5 = add nuw nsw i32 %i.018, 1
  %exitcond20.not = icmp eq i32 %inc5, %n
  br i1 %exitcond20.not, label %for.end6.loopexit, label %for.cond1.preheader

for.end6.loopexit:                                ; preds = %for.inc4
  br label %for.end6

for.end6:                                         ; preds = %for.end6.loopexit, %entry
  ret void
}

