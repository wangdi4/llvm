; RUN: opt -hir-details -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that we are able to eliminate load/store to %bound which is a region
; local alloca and then propagate %t to the loop upper and ztt via copy
; propagation and set its def@level as 1 in those refs

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %t = (%n)[0];
; CHECK: |   (%bound)[0] = %t;
; CHECK: |   %ld = (%bound)[0];
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, %ld + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   (@A)[0][i2] = i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After


; CHECK: + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %t = (%n)[0];
; CHECK: |   %ld = %t;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, %t + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   | <RVAL-REG> LINEAR i64 %t + -1{def@1}
; CHECK: |   |    <BLOB> LINEAR i64 %t{def@1}
; CHECK: |   | <ZTT-REG> LINEAR i64 %t{def@1}
; CHECK: |   |
; CHECK: |   |   (@A)[0][i2] = i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr %n) local_unnamed_addr #0 {
entry:
  %bound = alloca i64
  br label %for.outer

for.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %iv.outer.inc, %latch ]
  %t = load i64, ptr %n
  store i64 %t, ptr %bound, align 4
  %ld = load i64, ptr %bound, align 4
  %ztt.cmp = icmp sgt i64 %ld, 0
  br i1 %ztt.cmp, label %for.body.preheader, label %latch

for.body.preheader:                               ; preds = %for.outer
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv21 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next22, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i64], ptr @A, i64 0, i64 %indvars.iv21
  store i64 %indvars.iv21, ptr %arrayidx, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next22, %ld
  br i1 %exitcond24, label %for.exit, label %for.body

for.exit:
  br label %latch

latch:
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp = icmp eq i64 %iv.outer.inc, 100
  br i1 %cmp, label %for.end8, label %for.outer

for.end8:                                         ; preds = %for.end8.loopexit, %for.cond1.preheader
  ret void
}

