; RUN: opt < %s -analyze -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that the trip count estimate is conservatively evaluated to 10.
; Previously we evaluted it as 1 because the approximate max value of the blob
; %init was deduced to be 9 using the loop upper bound.

; CHECK: + DO i1 = 0, -1 * %init + 9, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   %ld = (%ptr)[0][i1 + %init];
; CHECK: + END LOOP


define void @foo([10 x i8]* %ptr, i64 %init) {
entry:
  br label %loop

loop:                                             ; preds = %entry, %latch
  %iv = phi i64 [ %iv.inc, %loop ], [ %init, %entry ]
  %gep = getelementptr inbounds [10 x i8], [10 x i8]* %ptr, i64 0, i64 %iv
  %ld = load i8, i8* %gep, align 1
  %iv.inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 10
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
