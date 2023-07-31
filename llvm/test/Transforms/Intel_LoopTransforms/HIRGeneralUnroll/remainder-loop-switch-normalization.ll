; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Incoming HIR-
; + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; |   (%ptr)[2 * i1] = 0;
; + END LOOP


; Verify that the remainder loop is successfully normalized for switch
; generation by creating a stand-alone blob from the lower bound and
; zero-extending it.

; CHECK:   %tgu = (%n + -1)/u8;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP> <nounroll>
; CHECK: |   (%ptr)[16 * i1] = 0;
; CHECK: |   (%ptr)[16 * i1 + 2] = 0;
; CHECK: |   (%ptr)[16 * i1 + 4] = 0;
; CHECK: |   (%ptr)[16 * i1 + 6] = 0;
; CHECK: |   (%ptr)[16 * i1 + 8] = 0;
; CHECK: |   (%ptr)[16 * i1 + 10] = 0;
; CHECK: |   (%ptr)[16 * i1 + 12] = 0;
; CHECK: |   (%ptr)[16 * i1 + 14] = 0;
; CHECK: + END LOOP

; CHECK: switch(%n + -8 * %tgu + -2)
; CHECK: {
; CHECK: case 6:
; CHECK:   L6.27:
; CHECK:   (%ptr)[2 * zext.i32.i64((8 * %tgu)) + 12] = 0;
; CHECK:   goto L5

define void @foo(i32 %n, ptr %ptr) {
entry:
  br label %loop

loop:                                              ; preds = %loop, %entry
  %iv.64 = phi i64 [ 0, %entry ], [ %iv.64.inc, %loop ]
  %iv.32 = phi i32 [ 1, %entry ], [ %iv.32.inc, %loop ]
  %gep = getelementptr inbounds i32, ptr %ptr, i64 %iv.64
  store i32 0, ptr %gep
  %iv.64.inc = add nuw nsw i64 %iv.64, 2
  %iv.32.inc = add nuw i32 %iv.32, 1
  %cmp = icmp ne i32 %iv.32.inc, %n
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}
