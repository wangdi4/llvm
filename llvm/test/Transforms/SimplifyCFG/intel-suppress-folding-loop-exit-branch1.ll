; RUN: opt < %s -S -simplifycfg | FileCheck %s

; Verify that in the presence of "pre_loopopt" attribute we do not combine loop
; exit condition %cmp1 with %cmp into an or condition to jump to common
; destination %exit.

; Note that IV update inst is used in the loop exit condition.

; CHECK-LABEL: @iv-update-preloopopt
; CHECK: loop:
; CHECK: latch:

define void @iv-update-preloopopt(i32 %v) "pre_loopopt" {
entry:
  br label %loop

loop:
  %iv.phi = phi i32 [ 0, %entry ], [ %iv.inc, %latch]
  %cmp = icmp eq i32 %v, 0
  br i1 %cmp, label %latch, label %exit

latch:
  %iv.inc = add nsw i32 %iv.phi, 1
  %cmp1 = icmp slt i32 %iv.inc, 15
  br i1 %cmp1, label %loop, label %exit

exit:
  ret void
}

; Verify that in the absence of "pre_loopopt" attribute, we combine %cmp and
; %cmp1 into an or condition.

; CHECK-LABEL: @iv-update-no-preloopopt
; CHECK: loop:
; CHECK-NOT: latch:
; CHECK: %or.cond
; CHECK: br i1 %or.cond, label %loop, label %exit

define void @iv-update-no-preloopopt(i32 %v) {
entry:
  br label %loop

loop:
  %iv.phi = phi i32 [ 0, %entry ], [ %iv.inc, %latch]
  %cmp = icmp eq i32 %v, 0
  br i1 %cmp, label %latch, label %exit

latch:
  %iv.inc = add nsw i32 %iv.phi, 1
  %cmp1 = icmp slt i32 %iv.inc, 15
  br i1 %cmp1, label %loop, label %exit

exit:
  ret void
}

