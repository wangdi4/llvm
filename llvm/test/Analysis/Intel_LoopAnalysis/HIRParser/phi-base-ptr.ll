; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that store with linear phi base is parsed correctly.
; CHECK: DO i1 = 0, 999
; CHECK-NEXT: (%p)[i1] = i1
; CHECK-NEXT: END LOOP



; ModuleID = 'ptr1_1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @sub(i32* nocapture %p) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.06 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %p.addr.05 = phi i32* [ %p, %entry ], [ %incdec.ptr, %for.body ]
  store i32 %i.06, i32* %p.addr.05, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.05, i64 1
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

