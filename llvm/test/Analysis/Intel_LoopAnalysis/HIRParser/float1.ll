; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %1 = *(i32*)(%B)[i1]
; CHECK-NEXT: *(i32*)(%A)[i1] = %1
; CHECK-NEXT: END LOOP

; ModuleID = 'float1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* nocapture %A, float* nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = bitcast float* %arrayidx2 to i32*
  store i32 %1, i32* %2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

