; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -passes='print<scalar-evolution>' -S 2>&1 | FileCheck %s

; Verify that the header phi with both operands as constant is treated as an AddRec if the backedge taken count of the loop is 1.

; CHECK: %iv.aux = phi i64 [ 0, %entry ], [ 1, %loop ]
; CHECK:  -->  {0,+,1}<nuw><nsw><%loop> U: [0,2) S: [0,2)

; CHECK: Loop %loop: backedge-taken count is 1

define void @foo(i8* %ptr) {
entry:
  br label %loop

loop:                                              ; preds = %entry, %loop
  %iv = phi i64 [ 0, %entry ], [ %inc, %loop ]
  %iv.aux = phi i64 [ 0, %entry ], [ 1, %loop ]
  %gep = getelementptr inbounds i8, i8* %ptr, i64 %iv
  store i8 0, i8* %gep, align 1
  %inc = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv, 1
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}


