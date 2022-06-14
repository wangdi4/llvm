; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 -disable-output | FileCheck %s

; Verify that store is parser in terms of base pointer %p even though the
; element type of %outer.ptr phi (i8) does not match element type of %inner.ptr (i64).

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   (%p)[%stride * i1 + i2] = 0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

define void @foo(ptr %p, i64 %stride) {
entry:
  %mul = mul i64 8, %stride
  br label %outer.loop

outer.loop:
  %outer.iv = phi i64 [ 0, %entry ], [ %outer.iv.inc, %outer.latch ]
  %outer.ptr = phi ptr [ %p, %entry ], [ %outer.ptr.inc, %outer.latch ]
  br label %inner.loop

inner.loop:
  %inner.iv = phi i64 [ 0, %outer.loop ], [ %inner.iv.inc, %inner.loop ]
  %inner.ptr = phi ptr [ %outer.ptr, %outer.loop ], [ %inner.ptr.inc, %inner.loop ]
  store i64 0, ptr %inner.ptr
  %inner.ptr.inc = getelementptr inbounds i64, ptr %inner.ptr, i64 1
  %inner.iv.inc = add i64 %inner.iv, 1
  %cmp = icmp eq i64 %inner.iv.inc, 10
  br i1 %cmp, label %outer.latch, label %inner.loop

outer.latch:
  %outer.ptr.inc = getelementptr inbounds i8, ptr %outer.ptr, i64 %mul
  %outer.iv.inc = add i64 %outer.iv, 1
  %cmp1 = icmp eq i64 %outer.iv.inc, 10
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  ret void
}
