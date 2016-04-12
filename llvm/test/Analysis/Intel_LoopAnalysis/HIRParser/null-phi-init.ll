; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the phi with null initial value is handled correcetly.
; CHECK: DO i1 = 0, (umax(4, %p) + -1)/u4
; CHECK-NEXT: %0 = {al:4}(null)[i1];
; CHECK-NEXT: END LOOP


; ModuleID = 'red.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define void @foo(i32* %p) {
entry:
  br label %for.body.i.i

for.body.i.i:                                     ; preds = %for.body.i.i, %entry
  %__q.013.i.i = phi i32* [ %incdec.ptr4.i.i, %for.body.i.i ], [ null, %entry ]
  %0 = load i32, i32* %__q.013.i.i, align 4
  %incdec.ptr4.i.i = getelementptr inbounds i32, i32* %__q.013.i.i, i64 1
  %cmp.i.i = icmp ult i32* %incdec.ptr4.i.i, %p
  br i1 %cmp.i.i, label %for.body.i.i, label %for.end

for.end:                                    ; preds = %for.body.i.i
  ret void
}

