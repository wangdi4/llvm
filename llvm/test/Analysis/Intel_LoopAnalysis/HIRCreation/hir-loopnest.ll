; RUN: opt < %s -analyze -hir-creation | FileCheck %s

; Check sequence of gotos/labels in output of hir-creation
; CHECK: for.cond1.preheader:
; CHECK: goto for.body3;
; CHECK-NEXT: for.body3:
; CHECK: goto if.then;
; CHECK-NEXT: if.then:
; CHECK: goto for.inc;
; CHECK: goto if.else;
; CHECK-NEXT: if.else:
; CHECK: goto for.inc;
; CHECK: for.inc
; CHECK: goto for.body3;
; CHECK: goto for.inc14;
; CHECK: goto for.inc14;
; CHECK: for.inc14
; CHECK: goto for.cond1.preheader;


; ModuleID = 'loopnest.c'; 
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x i32]] zeroinitializer, align 16
@B = common global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %m, i32 %n) {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.cond1.preheader.lr.ph, label %for.end16

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp223 = icmp sgt i32 %m, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc14, %for.cond1.preheader.lr.ph
  %indvars.iv27 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next28, %for.inc14 ]
  br i1 %cmp223, label %for.body3.preheader, label %for.inc14

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv27, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4
  %cmp6 = icmp sgt i32 %0, 5
  br i1 %cmp6, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  store i32 0, i32* %arrayidx5, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body3
  %arrayidx12 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx12, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %arrayidx12, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %m
  br i1 %exitcond, label %for.inc14.loopexit, label %for.body3

for.inc14.loopexit:                               ; preds = %for.inc
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond1.preheader
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %lftr.wideiv29 = trunc i64 %indvars.iv.next28 to i32
  %exitcond30 = icmp eq i32 %lftr.wideiv29, %n
  br i1 %exitcond30, label %for.end16.loopexit, label %for.cond1.preheader

for.end16.loopexit:                               ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.end16.loopexit, %entry
  ret void
}
