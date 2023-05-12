; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -hir-details 2>&1 < %s | FileCheck %s

; Verify that we are able to eliminate the very first store by also
; substituting the intermediate conditional store and load by the temp.
; Also verify that the temp use in load is set to non-linear.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = 10;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   if (%t == 0)
; CHECK: |   |   {
; CHECK: |   |      (%A)[0] = 2;
; CHECK: |   |   }
; CHECK: |   |   %ld = (%A)[0];
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   (%A)[0] = 5;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i64 i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %temp = 10;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   if (%t == 0)
; CHECK: |   |   {
; CHECK: |   |      %temp = 2;
; CHECK: |   |   }
; CHECK: |   |   %ld = %temp;
; CHECK: |   |   <RVAL-REG> NON-LINEAR i32 %temp
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   (%A)[0] = 5;
; CHECK: + END LOOP


define i32 @foo(ptr %A, i32 %t) {
entry:
  br label %loop.outer

loop.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %iv.outer.inc, %latch.outer ]
  store i32 10, ptr %A
  br label %loop

loop:
  %iv = phi i64 [ 0, %loop.outer ], [ %iv.inc, %latch ]
  %cmp = icmp eq i32 %t, 0
  br i1 %cmp, label %then, label %latch

then:
  store i32 2, ptr %A
  br label %latch

latch:
  %ld = load i32, ptr %A
  %iv.inc = add i64 %iv, 1
  %cmp1 = icmp eq i64 %iv.inc, 10
  br i1 %cmp1, label %latch.outer, label %loop

latch.outer:
  store i32 5, ptr %A
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp2 = icmp eq i64 %iv.outer.inc, 10
  br i1 %cmp2, label %exit, label %loop.outer

exit:
  %ld.ret = phi i32 [ %ld, %latch.outer ]
  ret i32 %ld.ret
}

