; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that a GEP (polynomial) blob is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %p.addr.07 = &((%p.addr.07)[i1])
; CHECK-NEXT: (%p.addr.07)[i1] = i1
; CHECK-NEXT: END LOOP

; ModuleID = 'poly-ptr1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %p, i32 %n) {
entry:
  %cmp.6 = icmp sgt i32 %n, 0
  br i1 %cmp.6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %p.addr.07 = phi i32* [ %add.ptr, %for.body ], [ %p, %entry ]
  %add.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %add.ptr, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

