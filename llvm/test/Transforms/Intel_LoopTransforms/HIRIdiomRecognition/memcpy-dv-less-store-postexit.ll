; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -disable-output 2>&1 < %s | FileCheck %s

; Verify that we can generate memcpy in the postexit in the presence of this
; edge-
; (%p)[i1 + 1] -> (%p)[i1] OUTPUT (<)


; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%p)[i1] = (%r)[i1];
; CHECK: |   %ld = (%q)[i1];
; CHECK: |   (%p)[i1 + 1] = %ld + 1;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = (%q)[i1];
; CHECK: |   (%p)[i1 + 1] = %ld + 1;
; CHECK: + END LOOP
; CHECK:     @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%p)[0]),  &((i8*)(%r)[0]),  100,  0);


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i8* nocapture %p, i8 *noalias %q, i8 *noalias %r) {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %iv = phi i64 [ %iv.inc, %while.body ], [ 0, %entry ]
  %iv.inc = add nsw i64 %iv, 1
  %gep1 = getelementptr inbounds i8, i8* %p, i64 %iv
  %gepinc1 = getelementptr inbounds i8, i8* %p, i64 %iv.inc
  %gep2 = getelementptr inbounds i8, i8* %q, i64 %iv
  %gep3 = getelementptr inbounds i8, i8* %r, i64 %iv
  %ld1 = load i8, i8* %gep3, align 8
  store i8 %ld1, i8* %gep1, align 8
  %ld = load i8, i8* %gep2, align 8
  %add = add i8 %ld, 1
  store i8 %add, i8* %gepinc1, align 8
  %cmp = icmp eq i64 %iv.inc, 100
  br i1 %cmp, label %while.end, label %while.body

while.end:                                        ; preds = %while.end.loopexit, %entry
  ret void
}

