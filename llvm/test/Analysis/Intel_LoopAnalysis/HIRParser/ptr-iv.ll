; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the pointer IV is handled correctly.
; CHECK: DO i1 = 0, (-1 * %p + %q + -4)/u4
; CHECK-NEXT: (%p)[2 * i1] = i1;
; CHECK-NEXT:  END LOOP


; ModuleID = 'ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* %p, i32* readnone %q) {
entry:
  %cmp.6 = icmp eq i32* %p, %q
  br i1 %cmp.6, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i32* %incdec.ptr, %q
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

