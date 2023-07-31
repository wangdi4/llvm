; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-multi-exit-loop-reroll,print<hir>" 2>&1 | FileCheck %s

; Verify that test case successfully compiled.
; There was an assert in hir-multi-exit-loop-reroll while trying to compare
; bitcast inst with double type and bitcast inst with integer type.
; It assumed that if the opcode of two instructions is the same and one of them
; is fpmath operator, the other instruction also has to be an fpmath operator.

; HIR-
; + DO i1 = 0, %iv.init + -1 * smin(0, %iv.init), 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483648>
; |   %fpred.out = %fpred;
; |   %i64red.out = %i64load * i1 + %i64.init;
; |   if (%i64load * i1 + %i64.init >=u %t3)
; |   {
; |      goto exit;
; |   }
; |   %fpupd = %fpred  +  %fpload;
; |   %i64upd = %i64load * i1 + %i64.init  +  %i64load;
; |   %fpred = %fpupd;
; + END LOOP

; CHECK-NOT: modified
; CHECK: DO i1

define void @foo(double %fpinit, i32 %iv.init, i64 %i64.init, i64 %t3, ptr %fpptr, ptr %i64ptr) {
entry:
  %fpload = load double, ptr %fpptr
  %i64load = load i64, ptr %i64ptr
  br label %loop

loop:                                             ; preds = %latch, %entry
  %iv = phi i32 [ %iv.inc, %latch ], [ %iv.init, %entry ]
  %i64red = phi i64 [ %i64upd, %latch ], [ %i64.init, %entry ]
  %fpred = phi double [ %fpupd, %latch ], [ %fpinit, %entry ]
  %t1975 = icmp ult i64 %i64red, %t3
  br i1 %t1975, label %latch, label %exit

latch:                                             ; preds = %loop
  %fpupd = fadd double %fpred, %fpload
  %i64upd = add i64 %i64red, %i64load
  %iv.inc = add i32 %iv, -1
  %t2000 = icmp sgt i32 %iv, 0
  br i1 %t2000, label %loop, label %exit

exit:                                             ; preds = %latch, %loop
  %t3695 = phi i64 [ %i64upd, %latch ], [ %i64red, %loop ]
  %t3696 = phi double [ %fpupd, %latch ], [ %fpred, %loop ]
  ret void
}
