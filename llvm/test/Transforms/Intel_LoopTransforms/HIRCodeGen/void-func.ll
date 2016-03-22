; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction -hir-cg -force-hir-cg -S | FileCheck %s

; Verify that the function call returning void is CG'd correctly.
; CHECK: region:
; CHECK: call void @bar

; ModuleID = 'void-func.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %A, i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  call void @bar(i32* %A, i32 %i.05) 
  %inc = add nuw nsw i32 %i.05, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

declare void @bar(i32*, i32) 


