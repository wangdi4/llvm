; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -print-after=hir-pre-vec-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that complete unroll does not assert when comparing non-equal refs
; (%p)[0].0 and (%p)[0] that they cannot have zero distance.

; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%p)[0].0 = %val;
; CHECK: |   %val = %val  +  (%p)[0];
; CHECK: + END LOOP


%struct = type { i32 } 

define void @foo(ptr %p, i32 %init) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %val = phi i32 [ %init, %entry ], [ %add, %loop ]
  %str.gep = getelementptr inbounds %struct, ptr %p, i64 0, i32 0
  store i32 %val, ptr %str.gep
  %ld = load i32, ptr %p
  %add = add i32 %val, %ld
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv, 4
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}


