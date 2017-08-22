; Test for Complete Unrolling with 2-level loops.
; Only the innermost loop should be unrolled as it has small trip count.

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-complete-unroll-loop-trip-threshold=50 -hir-cg -S < %s | FileCheck %s
; CHECK: entry

; terminator of entry bblock should point to new unrolled region.
; CHECK: for.cond1.preheader:
; CHECK: br i1 true, {{.*}}label %region

; check loop is completely unrolled.
; CHECK: region.0:
; CHECK: br label %[[LoopLabel:loop.[0-9]+]]
; CHECK: [[LoopLabel]]:
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

; Check for outer loop
; CHECK: icmp sle i64 %nextivloop.{{.*}}, 49
; CHECK: br i1 %condloop.{{.*}}, label %[[LoopLabel]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [550 x i32] zeroinitializer, align 16
@B = common global [550 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %b) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %i.019 = phi i64 [ 0, %entry ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.018 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %mul = shl nsw i64 %j.018, 1
  %add = add nuw nsw i64 %mul, %i.019
  %sub = add nsw i64 %add, -1
  %arrayidx = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %sub
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %add
  store i32 %0, i32* %arrayidx6, align 4
  %inc = add nuw nsw i64 %j.018, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i64 %i.019, 1
  %exitcond20 = icmp eq i64 %inc8, 50
  br i1 %exitcond20, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 
