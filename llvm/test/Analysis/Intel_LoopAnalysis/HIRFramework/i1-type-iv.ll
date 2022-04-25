; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that loops with i1 type IV are supported.

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK: |   %t61.out = i1;
; CHECK: + END LOOP

; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution 2>&1 | FileCheck %s --check-prefix=CHECK-SCEV
; RUN: opt < %s -passes="print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=CHECK-SCEV

; CHECK-SCEV:  %iv = phi i1 [ false, %loop ], [ true, %entry ]
; CHECK-SCEV:  -->  {true,+,true}<%loop> U: full-set S: full-set

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

