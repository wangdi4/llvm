; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 -disable-output | FileCheck %s

; Verify that %phi.ptr2 is parsed in inductive form even when it uses %phi.ptr1 as the update value.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   (%init.ptr)[2 * i1 + 2] = 0;
; CHECK: |   (%init.ptr)[2 * i1 + 3] = 1;
; CHECK: + END LOOP


define void @foo(ptr %init.ptr, i32 %n) {
entry:
  %init.add = getelementptr i8, ptr %init.ptr, i64 2
  br label %loop

loop:                                               ; preds = %loop, %entry
  %phi.ptr1 = phi ptr [ %phi.ptr1.inc, %loop ], [ %init.add, %entry ]
  %iv = phi i32 [ %iv.dec, %loop ], [ %n, %entry ]
  %phi.ptr2 = phi ptr [ %phi.ptr1, %loop ], [ %init.ptr, %entry ]
  %phi.ptr2.inc = getelementptr inbounds i8, ptr %phi.ptr2, i64 3
  store i8 0, ptr %phi.ptr1, align 1
  store i8 1, ptr %phi.ptr2.inc, align 1
  %iv.dec = add i32 %iv, -1
  %phi.ptr1.inc = getelementptr inbounds i8, ptr %phi.ptr1, i64 2
  %cmp = icmp eq i32 %iv.dec, 0
  br i1 %cmp, label %exit, label %loop

exit:                                               ; preds = %loop
  ret void
}
