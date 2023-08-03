; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -scalar-evolution-print-scoped-mode "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s --check-prefix=CHECK-SCOPED

; Verify that in scoped mode we are able to deduce max constant trip count in
; signed 32 bit range due to nowrap flag deduction on %add.

; CHECK:      %add = add nuw nsw i32 %n, 1
; CHECK-NEXT: -->  (1 + %n) U: full-set S: full-set
; CHECK: Loop %loop: constant max backedge-taken count is -1


; CHECK-SCOPED:       %add = add nuw nsw i32 %n, 1
; CHECK-SCOPED-NEXT:  -->  (1 + %n)<nuw><nsw> U: [1,0) S: [-2147483647,-2147483648)

; CHECK-SCOPED: Loop %loop: constant max backedge-taken count is 2147483646


define void @foo(i32 %n) {
entry:
  %cmp = icmp slt i32 %n, 1
  %add = add nuw nsw i32 %n, 1
  %wide.trip.count = sext i32 %add to i64
  br i1 %cmp, label %exit, label %preheader

preheader:                                    ; preds = %bb3
  br label %loop 

loop:                                              ; preds = %loop, %preheader
  %iv = phi i64 [ 1, %preheader ], [ %iv.next, %loop ]
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond117 = icmp eq i64 %iv.next, %wide.trip.count
  br i1 %exitcond117, label %loopexit, label %loop

loopexit:
  br label %exit

exit:
  ret void
}
