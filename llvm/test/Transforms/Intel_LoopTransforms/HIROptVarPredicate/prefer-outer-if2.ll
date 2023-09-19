; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that optimization happens for the condition number <24>
; since it is the second outermost condition in the loop and the condition
; number <7> is non-profitable.

; HIR before optimization
;<0>          BEGIN REGION { }
;<45>               + DO i1 = 0, 19, 1   <DO_LOOP>
;<46>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<7>                |   |   if (i2 >u %n)
;<7>                |   |   {
;<12>               |   |      if (i1 != i2)
;<12>               |   |      {
;<17>               |   |         (@a)[0][i1][i2] = i1;
;<12>               |   |      }
;<7>                |   |   }
;<22>               |   |   (@c)[0][i2][i1] = i2;
;<24>               |   |   if (i2 <u 4)
;<24>               |   |   {
;<29>               |   |      (@b)[0][i1][i2] = i2;
;<24>               |   |   }
;<46>               |   + END LOOP
;<45>               + END LOOP
;<0>          END REGION

; HIR after optimization
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   |   if (i2 >u %n)
; CHECK:           |   |   {
; CHECK:           |   |      if (i1 != i2)
; CHECK:           |   |      {
; CHECK:           |   |         (@a)[0][i1][i2] = i1;
; CHECK:           |   |      }
; CHECK:           |   |   }
; CHECK:           |   |   (@c)[0][i2][i1] = i2;
; CHECK:           |   |   (@b)[0][i1][i2] = i2;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   if (i2 + 4 >u %n)
; CHECK:           |   |   {
; CHECK:           |   |      if (i1 != i2 + 4)
; CHECK:           |   |      {
; CHECK:           |   |         (@a)[0][i1][i2 + 4] = i1;
; CHECK:           |   |      }
; CHECK:           |   |   }
; CHECK:           |   |   (@c)[0][i2 + 4][i1] = i2 + 4;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %0 = zext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc19
  %indvars.iv39 = phi i64 [ 0, %entry ], [ %indvars.iv.next40, %for.inc19 ]
  %1 = trunc i64 %indvars.iv39 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp ugt i64 %indvars.iv, %0
  br i1 %cmp4, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %for.body3
  %cmp5.not = icmp eq i64 %indvars.iv39, %indvars.iv
  br i1 %cmp5.not, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %arrayidx7 = getelementptr inbounds [20 x [20 x i32]], ptr @a, i64 0, i64 %indvars.iv39, i64 %indvars.iv
  store i32 %1, ptr %arrayidx7, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %for.body3
  %arrayidx11 = getelementptr inbounds [20 x [20 x i32]], ptr @c, i64 0, i64 %indvars.iv, i64 %indvars.iv39
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %arrayidx11, align 4
  %cmp12 = icmp ult i64 %indvars.iv, 4
  br i1 %cmp12, label %if.then13, label %for.inc

if.then13:                                        ; preds = %if.end
  %arrayidx17 = getelementptr inbounds [20 x [20 x i32]], ptr @b, i64 0, i64 %indvars.iv39, i64 %indvars.iv
  store i32 %2, ptr %arrayidx17, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end, %if.then13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond.not, label %for.inc19, label %for.body3

for.inc19:                                        ; preds = %for.inc
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41.not = icmp eq i64 %indvars.iv.next40, 20
  br i1 %exitcond41.not, label %for.end21, label %for.cond1.preheader

for.end21:                                        ; preds = %for.inc19
  ret i32 undef
}

