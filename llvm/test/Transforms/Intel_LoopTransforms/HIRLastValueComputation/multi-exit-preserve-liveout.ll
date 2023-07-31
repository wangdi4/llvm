; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation" -print-before=hir-last-value-computation -print-after=hir-last-value-computation -hir-details 2>&1 < %s | FileCheck %s

; Verify that we preserve %liveout as loop liveout even after the instruction
; %liveout = 10000; is moved to postexit because it is still liveout from the early
; exit via instruction %liveout = %inner.iv.out;

; CHECK: Dump Before

; CHECK: + DO i64 i1 = 0, 1999, 1   <DO_LOOP>

; CHECK: |   + LiveOut symbases: [[LIVEOUTSB:[0-9]+]]
; CHECK: |   + DO i32 i2 = 0, 9999, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   %inner.iv.out = i2;
; CHECK: |   |   %liveout = %inner.iv.out;
; CHECK: |   |      <LVAL-REG> NON-LINEAR i32 %inner.iv.out {sb:[[LIVEOUTSB]]}
; CHECK: |   |   if (%cond == 0)
; CHECK: |   |   {
; CHECK: |   |      goto outer.latch;
; CHECK: |   |   }
; CHECK: |   |   %liveout = 10000;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   outer.latch:
; CHECK: |   (%ptr)[i1 + %stride] = %liveout;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK:      + DO i64 i1 = 0, 1999, 1   <DO_LOOP>

; CHECK:      |   + LiveOut symbases: [[LIVEOUTSB]]
; CHECK:      |   + DO i32 i2 = 0, 9999, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:      |   |   %inner.iv.out = i2;
; CHECK:      |   |   %liveout = %inner.iv.out;
; CHECK:      |   |   if (%cond == 0)
; CHECK:      |   |   {
; CHECK-NEXT: |   |      goto outer.latch;
; CHECK:      |   |   }
; CHECK:      |   + END LOOP
; CHECK:      |      %liveout = 10000;
; CHECK:      |
; CHECK:      |   outer.latch:
; CHECK:      |   (%ptr)[i1 + %stride] = %liveout;
; CHECK:      + END LOOP


define void @foo(ptr %ptr, i64 %stride, i1 %cond) {
entry:
  br label %loop.outer

loop.outer:                                             ; preds = %outer.latch, %entry
  %outer.iv = phi i64 [ 0, %entry ], [ %outer.iv.inc, %outer.latch ]
  br label %loop.inner

loop.inner:                                             ; preds = %inner.latch, %loop.outer
  %inner.iv = phi i32 [ 0, %loop.outer ], [ %inner.iv.inc, %inner.latch ]
  br i1 %cond, label %inner.latch, label %outer.latch

inner.latch:                                             ; preds = %loop.inner
  %inner.iv.inc = add nuw nsw i32 %inner.iv, 1
  %i74 = icmp eq i32 %inner.iv.inc, 10000
  br i1 %i74, label %outer.latch, label %loop.inner

outer.latch:                                             ; preds = %inner.latch, %loop.inner
  %liveout = phi i32 [ %inner.iv, %loop.inner ], [ 10000, %inner.latch ]
  %i77 = add nuw nsw i64 %outer.iv, %stride
  %i78 = getelementptr inbounds i32, ptr %ptr, i64 %i77
  store i32 %liveout, ptr %i78, align 4
  %outer.iv.inc = add nuw nsw i64 %outer.iv, 1
  %i80 = icmp eq i64 %outer.iv.inc, 2000
  br i1 %i80, label %exit, label %loop.outer

exit:
  ret void
}
