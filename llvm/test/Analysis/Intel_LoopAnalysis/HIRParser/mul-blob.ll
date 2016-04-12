; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for loop verifying that multiplication blob is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %0 = {al:4}(@A1)[0][3 * i1 + 3 * %n]
; CHECK-NEXT: %add2 = %0  +  1.000000e+00
; CHECK-NEXT: {al:4}(@A1)[0][2 * i1 + %n] = %add2
; CHECK-NEXT: END LOOP


; ModuleID = 'p10.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A1 = global [1000 x float] zeroinitializer, align 16

define void @_Z3subl(i64 %n) {
entry:
  %cmp.13 = icmp sgt i64 %n, 0
  br i1 %cmp.13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.014 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %mul12 = add i64 %i.014, %n
  %add = mul i64 %mul12, 3
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @A1, i64 0, i64 %add
  %0 = load float, float* %arrayidx, align 4
  %add2 = fadd float %0, 1.000000e+00
  %mul3 = shl i64 %i.014, 1
  %add4 = add nsw i64 %mul3, %n
  %arrayidx5 = getelementptr inbounds [1000 x float], [1000 x float]* @A1, i64 0, i64 %add4
  store float %add2, float* %arrayidx5, align 4
  %inc = add nuw nsw i64 %i.014, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
