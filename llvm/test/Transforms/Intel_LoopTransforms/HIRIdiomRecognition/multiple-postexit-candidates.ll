; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -disable-output 2>&1 < %s | FileCheck %s

; Verify that we maintain lexical order for postexit candidates.


; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %add = (%p)[i1]  +  (%q)[i1];
; CHECK: |   (%p)[i1] = 0;
; CHECK: |   (%q)[i1] = (%r)[i1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %add = (%p)[i1]  +  (%q)[i1];
; CHECK: + END LOOP
; CHECK:    @llvm.memset.p0.i64(&((i8*)(%p)[0]),  0,  100,  0);
; CHECK:    @llvm.memcpy.p0.p0.i64(&((i8*)(%q)[0]),  &((i8*)(%r)[0]),  100,  0);


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %p, ptr noalias %q, ptr noalias %r) {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %iv = phi i64 [ %iv.inc, %while.body ], [ 0, %entry ]
  %iv.inc = add nsw i64 %iv, 1
  %gep1 = getelementptr inbounds i8, ptr %p, i64 %iv
  %gep2 = getelementptr inbounds i8, ptr %q, i64 %iv
  %gep3 = getelementptr inbounds i8, ptr %r, i64 %iv
  %ld1 = load i8, ptr %gep1, align 8
  %ld2 = load i8, ptr %gep2, align 8
  %add = add i8 %ld1, %ld2
  store i8 0, ptr %gep1, align 8
  %ld3 = load i8, ptr %gep3, align 8
  store i8 %ld3, ptr %gep2, align 8
  %cmp = icmp eq i64 %iv.inc, 100
  br i1 %cmp, label %while.end, label %while.body

while.end:                                        ; preds = %while.end.loopexit, %entry
  %add.lcssa = phi i8 [ %add, %while.body ]
  ret void
}

