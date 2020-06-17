; RUN: opt < %s -analyze -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that loops with i1 type IV are supported.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %t61.out = %t61;
; CHECK: |   %t61 = 1;
; CHECK: + END LOOP

define void @foo() {
entry:
  br label %loop

loop:                                               ; preds = %loop, %entry
  %iv = phi i1 [ false, %loop ], [ true, %entry ]
  %t61 = phi i64 [ 1, %loop ], [ 0, %entry ]
  br i1 %iv, label %loop, label %exit

exit:                                               ; preds = %loop
  %t61.lcssa = phi i64 [ %t61, %loop ]
  ret void
}

