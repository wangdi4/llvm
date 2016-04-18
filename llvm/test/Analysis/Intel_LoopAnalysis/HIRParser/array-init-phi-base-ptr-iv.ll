; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that ptr iv initialized using array element is parsed with the correct number of dimensions.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: {al:4}(@A)[0][i1 + 10] = i1
; CHECK-NEXT: END LOOP


; ModuleID = 'array-init-phi-base-ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [50 x i32] zeroinitializer, align 16

define void @foo(i32 %n) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.07 = phi i32* [ %incdec.ptr, %for.body ], [ getelementptr inbounds ([50 x i32], [50 x i32]* @A, i64 0, i64 10), %for.body.preheader ]
  store i32 %i.08, i32* %p.07, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.07, i64 1
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
