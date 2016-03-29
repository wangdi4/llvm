; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the pointer IV is handled correctly.
; CHECK: DO i1 = 0, (-1 * %p + %q + -4)/u4
; CHECK-NEXT: {al:4}(%p)[2 * i1] = i1;
; CHECK-NEXT:  END LOOP

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck -check-prefix=DETAIL %s
; Verify that we are able to detect no signed wrap for pointer IV loops.
; DETAIL: NSW: Yes

; ModuleID = 'ptr-iv.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %p, i32* readnone %q) {
entry:
  %cmp.6 = icmp eq i32* %p, %q
  br i1 %cmp.6, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %p.addr.07, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %p.addr.07, i64 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i32* %incdec.ptr, %q
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
