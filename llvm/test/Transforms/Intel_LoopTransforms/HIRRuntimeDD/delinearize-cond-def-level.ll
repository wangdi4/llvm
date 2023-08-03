; RUN: opt -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" -hir-details < %s 2>&1 | FileCheck %s

; Check def levels inside delinearization conditions.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%p)[i1];
;       |
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   %1 = (%q)[i2];
;       |   |   (%p)[%0 * i1 + i2 + 1] = %1 + 1;
;       |   |   (%p)[%0 * i1 + i2 + %0] = 1;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK:      if (%0 > 1 & 10 < %0
; CHECK-NEXT: <RVAL-REG> NON-LINEAR i64 %0 {sb:6}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %p, ptr nocapture readonly %q, i64 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %indvars.iv37 = phi i64 [ 0, %entry ], [ %indvars.iv.next38, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds i64, ptr %p, i64 %indvars.iv37
  %0 = load i64, ptr %arrayidx, align 8
  %mul = mul nsw i64 %0, %indvars.iv37
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond39 = icmp eq i64 %indvars.iv.next38, 100
  br i1 %exitcond39, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx6 = getelementptr inbounds i64, ptr %q, i64 %indvars.iv
  %1 = load i64, ptr %arrayidx6, align 8
  %add = add nsw i64 %1, 1
  %add8 = add nsw i64 %mul, %indvars.iv
  %add9 = add nsw i64 %add8, 1
  %arrayidx10 = getelementptr inbounds i64, ptr %p, i64 %add9
  store i64 %add, ptr %arrayidx10, align 8
  %add15 = add nsw i64 %add8, %0
  %arrayidx16 = getelementptr inbounds i64, ptr %p, i64 %add15
  store i64 1, ptr %arrayidx16, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

