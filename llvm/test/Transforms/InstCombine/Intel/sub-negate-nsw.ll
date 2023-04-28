; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; Verify that nsw flag is retained on the add instruction formed by negating
; sub instruction.

; CHECK: %t2.neg = phi i32 [ 0, %bb1 ], [ -1, %bb2 ]
; CHECK: %sub = add nsw i32 %t2.neg, %t3


define i32 @foo(i32 %t1, i32 %t3) {
entry:
  %cmp = icmp sgt i32 %t1, 0
  br i1 %cmp, label %bb1, label %bb2

bb1:
  br label %merge

bb2:
  br label %merge

merge:
  %t2 = phi i32 [ 0, %bb1 ], [ 1, %bb2 ]
  %sub = sub nsw i32 %t3, %t2
  ret i32 %sub
}
