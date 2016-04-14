; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that store with indirect linear phi base is parsed correctly.
; CHECK: DO i1 = 0, 999
; CHECK-NEXT: {al:4}(%p)[i1 + 1] = i1
; CHECK-NEXT: END LOOP


; ModuleID = 'ptr4_1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @sub(i32* nocapture %p) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %p.addr.05 = phi i32* [ %p, %entry ], [ %incdec.ptr, %for.body ]
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.05, i64 1
  store i32 %i.06, i32* %incdec.ptr, align 4
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

