; RUN: opt < %s -passes='loop-vectorize' -S 2>&1 | FileCheck %s

; Verify that the test case compiles successfully.
; Vectorizer was choking when treating %iv.aux with both operands as constants
; as an induction phi.

; CHECK: @foo

define void @foo(ptr %ptr) {
entry:
  br label %loop

loop:                                              ; preds = %entry, %loop
  %iv = phi i64 [ 0, %entry ], [ %inc, %loop ]
  %iv.aux = phi i64 [ 0, %entry ], [ 1, %loop ]
  %gep = getelementptr inbounds i8, ptr %ptr, i64 %iv
  store i8 0, ptr %gep, align 1
  %inc = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv, 1
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}


