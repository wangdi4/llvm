; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the max trip count is picked up from ScalarEvolution.

; CHECK: + DO i1 = 0, 2 * %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>
; CHECK: |   (%A)[i1 + 1] = i1 + 1;
; CHECK: + END LOOP


; ModuleID = 'scev-max-tc.ll'
source_filename = "scev-max-tc.ll"

define void @even(i4 %n, ptr %A) {
entry:
  %m = shl i4 %n, 1
  %s = icmp sgt i4 %m, 0
  br i1 %s, label %loop.preheader, label %exit

loop.preheader:                                   ; preds = %entry
  br label %loop

loop:                                             ; preds = %loop.preheader, %loop
  %i = phi i4 [ %i.next, %loop ], [ 0, %loop.preheader ]
  %i.next = add i4 %i, 1
  %indvars.iv = zext i4 %i.next to i64
  %arrayidx = getelementptr inbounds i4, ptr %A, i64 %indvars.iv
  store i4 %i.next, ptr %arrayidx
  %t = icmp slt i4 %i.next, %m
  br i1 %t, label %loop, label %exit.loopexit

exit.loopexit:                                    ; preds = %loop
  br label %exit

exit:                                             ; preds = %exit.loopexit, %entry
  ret void
}
