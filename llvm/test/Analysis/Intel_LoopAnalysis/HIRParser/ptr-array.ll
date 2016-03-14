; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Source code-
; int A[10][10];
; int (*B[5])[10];
;
; void foo(int n) {
;   int i;
;
;   for (i=0; i<5; i++) {
;     B[i] = &A[i];
;   }
; }

; Check parsing output for the loop verifying that the GEP of @A which has less number of indices than the maximum number possible (3), is parsed correctly.
; CHECK: DO i1 = 0, 4
; CHECK-NEXT: (@B)[0][i1] = &((@A)[0][i1])
; CHECK-NEXT: END LOOP


; ModuleID = 'ptr-array.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [10 x [10 x i32]] zeroinitializer, align 16
@B = common global [5 x [10 x i32]*] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 %idxprom
  %idxprom1 = sext i32 %i.01 to i64
  %arrayidx2 = getelementptr inbounds [5 x [10 x i32]*], [5 x [10 x i32]*]* @B, i64 0, i64 %idxprom1
  store [10 x i32]* %arrayidx, [10 x i32]** %arrayidx2, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

