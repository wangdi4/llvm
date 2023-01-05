; RUN: opt -passes="jump-threading" < %s -S | FileCheck %s

; Verify that jump threading is suppressed for countable loop backedges prior to loopopt("pre_loopopt").

; CHECK: latch:
; CHECK:  br i1 %exitcond1065.i, label %red.cmp, label %loop
; CHECK-NOT: latch.thread;

define void @foo(i32 %n) "pre_loopopt" {
entry:
  %rel.i = icmp slt i32 %n, 1
  br label %preheader

preheader:                                            ; preds = %entry
  br label %loop

loop:                                          ; preds = %latch, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %latch ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %rel.i, label %latch, label %bb538.preheader.i

bb538.preheader.i:                                ; preds = %loop
  %exitcond1060.i = icmp eq i64 undef, undef
  br label %latch

latch:                                          ; preds = %bb538.preheader.i, %loop
  %exitcond1065.i = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond1065.i, label %exit, label %loop

exit:                              ; preds = %latch
  br label %red.cmp

red.cmp:                                  ; preds = %exit
  br i1 %rel.i, label %ret2, label %ret1

ret1:                              ; preds = %red.cmp
  ret void

ret2:                               ; preds = %red.cmp
  ret void
}

; Verify that jump threading kicks in without "pre_loopopt" attribute.
; CHECK: foo1
; CHECK: latch:                                          ; preds = %loop
; CHECK: br i1 %exitcond1065.i, label %ret2, label %loop

; CHECK: latch.thread:                                   ; preds = %loop
; CHECK: br i1 %exitcond1065.i1, label %ret1, label %loop

define void @foo1(i32 %n) {
entry:
  %rel.i = icmp slt i32 %n, 1
  br label %preheader

preheader:                                            ; preds = %entry
  br label %loop

loop:                                          ; preds = %latch, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %latch ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %rel.i, label %latch, label %bb538.preheader.i

bb538.preheader.i:                                ; preds = %loop
  %exitcond1060.i = icmp eq i64 undef, undef
  br label %latch

latch:                                          ; preds = %bb538.preheader.i, %loop
  %exitcond1065.i = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond1065.i, label %exit, label %loop

exit:                              ; preds = %latch
  br label %red.cmp

red.cmp:                                  ; preds = %exit
  br i1 %rel.i, label %ret2, label %ret1

ret1:                              ; preds = %red.cmp
  ret void

ret2:                               ; preds = %red.cmp
  ret void
}
