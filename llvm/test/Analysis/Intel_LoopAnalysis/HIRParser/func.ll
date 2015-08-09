; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop
; CHECK: DO i1 = 0
; CHECK-NEXT: @bar(i1)
; CHECK-NEXT: END LOOP


; ModuleID = 'func.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  tail call void @bar(i32 %i.05) 
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare void @bar(i32)
