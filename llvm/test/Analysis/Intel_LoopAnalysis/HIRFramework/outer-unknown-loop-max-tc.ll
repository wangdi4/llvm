; RUN: opt < %s -hir-cost-model-throttling=0 -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that the the test compiles successfully. It was failing in max trip
; count estimation because it tried to access upper bound of outer unknown loop
; while dealing with the reference (@A)[0][i1 + i2].

; CHECK: + UNKNOWN LOOP i1  <MAX_TC_EST = 50>
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %0 = (%B)[0];
; CHECK: |
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50>
; CHECK: |   |   (@A)[0][i1 + i2] = %0 + -1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   if (i1 + 1 < %0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [50 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* %B, i64 %n) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i64 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %0 = load i32, i32* %B, align 4
  br label %for.inner

for.inner:                                          ; preds = %for.body
  %j = phi i64 [ %j.inc, %for.inner ], [ 0, %for.body ]
  %add = add nsw i64 %indvars.iv, %j
  %arrayidx = getelementptr inbounds [50 x i32], [50 x i32]* @A, i64 0, i64 %add
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %arrayidx, align 4
  %j.inc =  add nuw nsw i64 %j, 1
  %cmp2 = icmp slt i64 %j.inc, %n
  br i1 %cmp2, label %for.inner, label %for.inc

for.inc:                                          ; preds = %for.body, %for.inner
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %sext = sext i32 %0 to i64
  %cmp = icmp slt i64 %indvars.iv.next, %sext
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

