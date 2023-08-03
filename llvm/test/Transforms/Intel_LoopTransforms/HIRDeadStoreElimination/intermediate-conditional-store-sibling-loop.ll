; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -hir-details 2>&1 < %s | FileCheck %s

; Verify that we are able to eliminate the very first store by also
; substituting the intermediate conditional store and load by the temp.
; Also verify that temp is marked as def@1 for the use in load.

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = 10;
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   if (%t == 0)
; CHECK: |   |   {
; CHECK: |   |      (%A)[0] = 2;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
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
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   %ld = %temp;
; CHECK: |   |   <RVAL-REG> LINEAR i32 %temp{def@1}
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
  %iv.inc = add i64 %iv, 1
  %cmp1 = icmp eq i64 %iv.inc, 10
  br i1 %cmp1, label %loop2.pre, label %loop

loop2.pre:
  br label %loop2

loop2:
  %iv2 = phi i64 [ 0, %loop2.pre ], [ %iv2.inc, %loop2 ]
  %iv2.inc = add i64 %iv2, 1
  %ld = load i32, ptr %A
  %cmp2 = icmp eq i64 %iv2.inc, 10
  br i1 %cmp2, label %latch.outer, label %loop2

latch.outer:
  %ld.lcssa = phi i32 [ %ld, %loop2]
  store i32 5, ptr %A
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp.outer = icmp eq i64 %iv.outer.inc, 10
  br i1 %cmp.outer, label %exit, label %loop.outer

exit:
  %ld.ret = phi i32 [ %ld.lcssa, %latch.outer ]
  ret i32 %ld.ret
}

