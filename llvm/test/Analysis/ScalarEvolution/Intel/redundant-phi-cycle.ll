; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that %red.phi and %merge.phi which are part of redundant phi cycle are
; parsed in terms of %init.

; CHECK: %red.phi = phi i32 [ %init, %entry ], [ %merge.phi, %latch ]
; CHECK-NEXT:  -->  %init

; CHECK: %merge.phi = phi i32 [ %red.phi, %loop.outer ], [ %init, %loop.inner ]
; CHECK-NEXT:  -->  %init

define i32 @foo(i32 %init, i1 %cond) {
entry:
  br label %loop.outer

loop.outer:
  %iv.outer = phi i32 [ 0, %entry ], [ %iv.outer.inc, %latch ]
  %red.phi = phi i32 [ %init, %entry ], [ %merge.phi, %latch ]
  br i1 %cond, label %loop.inner, label %latch

loop.inner:
  %iv.inner = phi i32 [ 0, %loop.outer ], [ %iv.inner.inc, %loop.inner ]
  %iv.inner.inc = add i32 %iv.inner, 1
  %cmp = icmp eq i32 %iv.inner.inc, 5
  br i1 %cmp, label %latch, label %loop.inner

latch:
  %merge.phi = phi i32 [ %red.phi, %loop.outer ], [ %init, %loop.inner ]
  %iv.outer.inc = add i32 %iv.outer, 1
  %cmp1 = icmp eq i32 %iv.outer.inc, 5
  br i1 %cmp1, label %exit, label %loop.outer

exit:
  %lcssa = phi i32 [ %merge.phi, %latch ]
  ret i32 %lcssa
}
