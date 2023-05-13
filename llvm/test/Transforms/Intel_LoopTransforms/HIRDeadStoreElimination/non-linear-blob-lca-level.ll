; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we are able to optimize away store to (%A)[0][i2 + %ld] in the
; first i2 loop even though %ld is defined at LCA level of the refs making it
; non-linear at that level.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %ld = (%B)[0];
; CHECK: |
; CHECK: |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK: |   |   (%A)[0][i2 + %ld] = 10;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK: |   |   (%A)[0][i2 + %ld] = 20;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: modified

; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT: |   %ld = (%B)[0];
; CHECK-NEXT: |
; CHECK-NEXT: |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT: |   |   (%A)[0][i2 + %ld] = 20;
; CHECK-NEXT: |   + END LOOP
; CHECK-NEXT: + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr %A, ptr %B) {
entry:
  br label %loop.outer

loop.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %iv.outer.inc, %latch.outer ]
  %ld = load i64, ptr %B
  br label %loop1

loop1:
  %iv1 = phi i64 [ 0, %loop.outer ], [ %iv1.inc, %loop1 ]
  %add1 = add i64 %ld, %iv1
  %gep1 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %add1
  store i32 10, ptr %gep1, align 4
  %iv1.inc = add i64 %iv1, 1
  %cmp1 = icmp eq i64 %iv1.inc, 4
  br i1 %cmp1, label %loop.exit, label %loop1

loop.exit:
  br label %loop2

loop2:
  %iv2 = phi i64 [ 0, %loop.exit ], [ %iv2.inc, %loop2 ]
  %add2 = add i64 %ld, %iv2
  %gep2 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %add2
  store i32 20, ptr %gep2, align 4
  %iv2.inc = add i64 %iv2, 1
  %cmp2 = icmp eq i64 %iv2.inc, 4
  br i1 %cmp2, label %latch.outer, label %loop2

latch.outer:
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp.outer = icmp eq i64 %iv.outer.inc, 4
  br i1 %cmp.outer, label %exit, label %loop.outer

exit:
  ret void
}

declare ptr @bar() #0

attributes #0 = { nounwind memory(none) }
