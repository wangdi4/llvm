; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output -hir-cost-model-throttling=0 2>&1 | FileCheck %s

; Verify that we are able to get rid of the liveout copies in the outer loop.

; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, sext.i32.i64((-1 + %m)), 1   <DO_LOOP>
; CHECK: |   |   %0 = (@A)[0][i2][i1];
; CHECK: |   |   %t.034 = %0  +  %t.034;
; CHECK: |   |   %t.034.out = %t.034;
; CHECK: |   |   %1 = (@B)[0][i2][i1];
; CHECK: |   |   %t.034 = %t.034  +  %1;
; CHECK: |   |   %call = @foo1(%t.034.out);
; CHECK: |   + END LOOP
; CHECK: |      %r.035 = %t.034.out;
; CHECK: |
; CHECK: |   %t.034.out1 = %t.034;
; CHECK: |   %r.035.out = %r.035;
; CHECK: + END LOOP


; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   + DO i2 = 0, sext.i32.i64((-1 + %m)), 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; CHECK: |   |   %t.034 = (@A)[0][i2][i1]  +  %t.034;
; CHECK: |   |   %t.034.out = %t.034;
; CHECK: |   |   %t.034 = %t.034  +  (@B)[0][i2][i1];
; CHECK: |   |   %call = @foo1(%t.034.out);
; CHECK: |   + END LOOP
; CHECK: |      %r.035 = %t.034.out;
; CHECK: + END LOOP


; ModuleID = 'red_nest2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x i32]] zeroinitializer, align 16
@B = common global [1000 x [1000 x i32]] zeroinitializer, align 16

define void @foo(i32 %n, i32 %m) {
entry:
  %cmp32 = icmp sgt i32 %n, 0
  br i1 %cmp32, label %for.cond1.preheader.lr.ph, label %for.end13

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp228 = icmp sgt i32 %m, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %for.cond1.preheader.lr.ph
  %indvars.iv38 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next39, %for.inc11 ]
  %r.035 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %r.1.lcssa, %for.inc11 ]
  %t.034 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %t.1.lcssa, %for.inc11 ]
  br i1 %cmp228, label %for.body3.preheader, label %for.inc11

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %t.130 = phi i32 [ %add10, %for.body3 ], [ %t.034, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x i32]], ptr @A, i64 0, i64 %indvars.iv, i64 %indvars.iv38
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, %t.130
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv38
  %1 = load i32, ptr %arrayidx9, align 4
  %add10 = add nsw i32 %add, %1
  %call = tail call i32 @foo1(i32 %add)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.inc11.loopexit, label %for.body3

for.inc11.loopexit:                               ; preds = %for.body3
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.cond1.preheader
  %r.1.lcssa = phi i32 [ %r.035, %for.cond1.preheader ], [ %add, %for.inc11.loopexit ]
  %t.1.lcssa = phi i32 [ %t.034, %for.cond1.preheader ], [ %add10, %for.inc11.loopexit ]
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %lftr.wideiv40 = trunc i64 %indvars.iv.next39 to i32
  %exitcond41 = icmp eq i32 %lftr.wideiv40, %n
  br i1 %exitcond41, label %for.end13.loopexit, label %for.cond1.preheader

for.end13.loopexit:                               ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %entry
  %r.0.lcssa = phi i32 [ 1, %entry ], [ %r.1.lcssa, %for.end13.loopexit ]
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.end13.loopexit ]
  %add14 = add nsw i32 %r.0.lcssa, %t.0.lcssa
  store i32 %add14, ptr getelementptr inbounds ([1000 x [1000 x i32]], ptr @A, i64 0, i64 5, i64 5), align 4
  ret void
}

declare i32 @foo1(i32)
