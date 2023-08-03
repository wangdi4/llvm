; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we don't perform unroll & jam for conditional memory refs as they are not profitable.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   if (i2 > 5)
; CHECK: |   |   {
; CHECK: |   |      (@A)[0][i2] = i1 + i2;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.cond1.preheader.lr.ph, label %for.end8

for.cond1.preheader.lr.ph:                        ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc6
  %i.04 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc7, %for.inc6 ]
  %cmp21 = icmp slt i32 0, %n
  br i1 %cmp21, label %for.body3.lr.ph, label %for.inc6

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %latch
  %j.02 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %latch ]
  %cmp4 = icmp sgt i32 %j.02, 5
  br i1 %cmp4, label %if, label %latch

if:
  %idxprom = sext i32 %j.02 to i64
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %add = add nsw i32 %i.04, %j.02
  store i32 %add, ptr %arrayidx5, align 4
  br label %latch

latch:
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %n
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc6_crit_edge

for.cond1.for.inc6_crit_edge:                     ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.cond1.for.inc6_crit_edge, %for.cond1.preheader
  %inc7 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc7, %n
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.for.end8_crit_edge

for.cond.for.end8_crit_edge:                      ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.cond.for.end8_crit_edge, %entry
  ret void
}
