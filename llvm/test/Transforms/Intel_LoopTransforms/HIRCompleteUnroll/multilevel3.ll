; Test for Complete Unrolling for 3 loops which have 2-level nesting.
; All the loops should be unrolled as they have small trip count.
; The pass should generate just the region with statements.

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -S < %s | FileCheck %s
; CHECK: entry

; terminator of entry bblock should point to new unrolled region.
; CHECK: for.cond{{.*}}.preheader:
; CHECK: br i1 true, {{.*}}label %region

; check loop is completely unrolled.
; CHECK: region.0:
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 -1)
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 0)
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 3)
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 2)
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 3)
; CHECK-NEXT: br label %for.end{{.*}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [550 x i32] zeroinitializer, align 16
@B = common global [550 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %b) {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.16, %entry
  %i.033 = phi i64 [ 0, %entry ], [ %inc17, %for.inc.16 ]
  br label %for.body.3

for.cond.7.preheader:                             ; preds = %for.body.3
  %.pre = load i32, i32* getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 0), align 16
  br label %for.body.9

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.031 = phi i64 [ 0, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %mul = shl nsw i64 %j.031, 1
  %add = add nuw nsw i64 %mul, %i.033
  %sub = add nsw i64 %add, -1
  %arrayidx = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %sub
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %add
  store i32 %0, i32* %arrayidx6, align 4
  %inc = add nuw nsw i64 %j.031, 1
  %exitcond = icmp eq i64 %inc, 3
  br i1 %exitcond, label %for.cond.7.preheader, label %for.body.3

for.body.9:                                       ; preds = %for.body.9, %for.cond.7.preheader
  %k.032 = phi i64 [ 0, %for.cond.7.preheader ], [ %add11, %for.body.9 ]
  %add11 = add nuw nsw i64 %k.032, 1
  %arrayidx12 = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %add11
  store i32 %.pre, i32* %arrayidx12, align 4
  %exitcond34 = icmp eq i64 %add11, 3
  br i1 %exitcond34, label %for.inc.16, label %for.body.9

for.inc.16:                                       ; preds = %for.body.9
  %inc17 = add nuw nsw i64 %i.033, 1
  %exitcond35 = icmp eq i64 %inc17, 2
  br i1 %exitcond35, label %for.end.18, label %for.cond.1.preheader

for.end.18:                                       ; preds = %for.inc.16
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 

