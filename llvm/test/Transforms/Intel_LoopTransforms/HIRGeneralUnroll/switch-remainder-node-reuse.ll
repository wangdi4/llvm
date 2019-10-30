; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; This simple switch generation test was failing when we tried to reuse nodes
; from the original loop because of a bug in HLNodeUtills::remove().
; This test verifies that the utility bug is fixed.

; Incoming HIR-
; + DO i1 = 0, umax(1, %n) + -1, 1   <DO_LOOP>
; |   %t17 = (%ptr)[i1];
; |   %t13 = (%t17)/u8  +  %t13;
; + END LOOP

; CHECK: switch(umax(1, %n) + -8 * %tgu + -1)
; CHECK: {
; CHECK: case 6:
; CHECK:    L6.36:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 6];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L5.39;
; CHECK: case 5:
; CHECK:    L5.39:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 5];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L4.43;
; CHECK: case 4:
; CHECK:    L4.43:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 4];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L3.47;
; CHECK: case 3:
; CHECK:    L3.47:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 3];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L2.51;
; CHECK: case 2:
; CHECK:    L2.51:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 2];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L1.55;
; CHECK: case 1:
; CHECK:    L1.55:
; CHECK:    %t17 = (%ptr)[8 * %tgu + 1];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    goto L0.59;
; CHECK: case 0:
; CHECK:    L0.59:
; CHECK:    %t17 = (%ptr)[8 * %tgu];
; CHECK:    %t13 = (%t17)/u8  +  %t13;
; CHECK:    break;
; CHECK: default:
; CHECK:    break;
; CHECK: }


define void @foo(i64* %ptr, i64 %n) {
entry:
  br label %loop

loop:                                               ; preds = %loop, %entry
  %ptr.phi = phi i64* [ %ptr, %entry ], [ %ptr.inc, %loop ]
  %t12 = phi i64 [ 0, %entry ], [ %t20, %loop ]
  %t13 = phi i64 [ 0, %entry ], [ %t19, %loop ]
  %ptr.inc = getelementptr i64, i64* %ptr.phi, i64 1
  %t17 = load i64, i64* %ptr.phi, align 8
  %t18 = lshr i64 %t17, 3
  %t19 = add i64 %t18, %t13
  %t20 = add nuw nsw i64 %t12, 1
  %t21 = icmp ult i64 %t20, %n
  br i1 %t21, label %loop, label %exit

exit:                                               ; preds = %loop
  %t23 = phi i64 [ %t19, %loop ]
  ret void
}
