; Test for General unrolling with if condition.

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s
; HIR Check
; CHECK: BEGIN REGION { modified }
; Check unrolling of loop.
; CHECK: DO i1 = 0, 123, 1
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if
; CHECK: if (8 * i1 + 8 > 1)
; CHECK: END LOOP
; CHECK: DO i1 = 992, 998, 1
; CHECK: if (i1 + 1 > 1)
; CHECK: END LOOP
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x i32] zeroinitializer, align 16
@B = common global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %k) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %j.07 = phi i64 [ 1, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp sgt i64 %j.07, 1
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %j.07
  store i32 1, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %j.07, 1
  %exitcond = icmp eq i64 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 


