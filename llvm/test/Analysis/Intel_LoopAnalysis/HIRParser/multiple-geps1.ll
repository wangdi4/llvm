; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the load whose address is formed using multiple geps is parsed correctly.

; CHECK: DO i1 = 0, 99
; CHECK-NEXT: (%0)[i1 + 2] = i1
; CHECK-NEXT: END LOOP


; ModuleID = 't1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external global ptr, align 8

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  %0 = load ptr, ptr @A, align 8
  %add.ptr = getelementptr inbounds i32, ptr %0, i64 2
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %add.ptr, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

