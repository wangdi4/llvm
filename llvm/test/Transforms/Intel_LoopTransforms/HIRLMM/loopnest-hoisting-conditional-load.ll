; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-lmm,print<hir>" -aa-pipeline="basic-aa" -hir-lmm-loopnest-hoisting=true -hir-details < %s 2>&1 | FileCheck %s

; Verify that we hoist the conditional load (%B)[2] in loopnest hoisting mode
; because of the identical region dominating load %0.

; Dump Before

; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   %div = %n  /  %t;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   |   if (%t + %0 > 3)
; CHECK: |   |   {
; CHECK: |   |      %1 = (%B)[2];
; CHECK: |   |      (@A)[0][i1][i2] = %1 + %div;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; Dump After

; CHECK: BEGIN REGION { modified }
; CHECK:   %1 = (%B)[2];
; CHECK: + DO i64 i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   %div = %n  /  %t;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   |   if (%t + %0 > 3)
; CHECK: |   |   {
; CHECK: |   |      (@A)[0][i1][i2] = %1 + %div;

; Verify that definition levels are updated correctly after hoisting.

; CHECK: |   |      <RVAL-REG> LINEAR i32 %1 + %div{def@1} {sb:2}
; CHECK: |   |         <BLOB> LINEAR i32 %div{def@1} {sb:5}
; CHECK: |   |         <BLOB> LINEAR i32 %1 {sb:11}
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16
; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32 %n, i32 %t, ptr noalias readonly %B) local_unnamed_addr #0 {
entry:
  %gep = getelementptr inbounds i32, ptr %B, i64 2
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.cond1.preheader.lr.ph, label %for.end12

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = load i32, ptr %gep, align 4
  %add = add nsw i32 %0, %t
  %cmp4 = icmp sgt i32 %add, 3
  %wide.trip.count3032 = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.inc10
  %indvars.iv28 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next29, %for.inc10 ]
  %div = sdiv i32 %n, %t
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %1 = load i32, ptr %gep, align 4
  %arrayidx8 = getelementptr inbounds [10 x [10 x i32]], ptr @A, i64 0, i64 %indvars.iv28, i64 %indvars.iv
  %add9 = add nsw i32 %div, %1
  store i32 %add9, ptr %arrayidx8, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count3032
  br i1 %exitcond, label %for.inc10, label %for.body3

for.inc10:                                        ; preds = %for.inc
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next29, %wide.trip.count3032
  br i1 %exitcond31, label %for.end12.loopexit, label %for.body3.lr.ph

for.end12.loopexit:                               ; preds = %for.inc10
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}
