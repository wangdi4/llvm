; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; This test was asserting because we failed to extract the constant multiplier
; of 2 from the blob (zext.i16.i32(%t11) * ((2 * %t4) +  (2 * %t6))) which is
; the i1 coeff of the load.

; CHECK: + DO i1 = 0, %t5 + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %t4 + -1, 1   <DO_LOOP>
; CHECK: |   |   %t32 = (%t19)[(zext.i16.i32(%t11) * (%t4 + %t6)) * i1 + zext.i16.i32(%t11) * i2];
; CHECK: |   |   (%t1)[(%t4 + %t7) * i1 + i2] = %t32;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define hidden void @putRGBAAcontig16bittile(ptr %t10, ptr %t1, i32 %t4, i32 %t5, i32 %t6, i32 %t7, ptr %t19) {
t9:
  %t11 = load i16, ptr %t10, align 2
  %t12 = zext i16 %t11 to i32
  %t13 = mul nsw i32 %t12, %t6
  br label %t17

t17:                                               ; preds = %t9
  %t20 = mul i32 %t12, %t4
  br label %t22

t22:                                               ; preds = %t64, %t17
  %t23 = phi i32 [ %t26, %t64 ], [ %t5, %t17 ]
  %t24 = phi ptr [ %t68, %t64 ], [ %t19, %t17 ]
  %t25 = phi ptr [ %t67, %t64 ], [ %t1, %t17 ]
  %t26 = add i32 %t23, -1
  br label %t27

t27:                                               ; preds = %t27, %t22
  %t28 = phi ptr [ %t24, %t22 ], [ %t62, %t27 ]
  %t29 = phi ptr [ %t25, %t22 ], [ %t61, %t27 ]
  %t30 = phi i32 [ %t4, %t22 ], [ %t31, %t27 ]
  %t31 = add i32 %t30, -1
  %t32 = load i16, ptr %t28, align 2
  %t33 = zext i16 %t32 to i32
  store i32 %t33, ptr %t29, align 4
  %t61 = getelementptr inbounds i32, ptr %t29, i32 1
  %t62 = getelementptr inbounds i16, ptr %t28, i32 %t12
  %t63 = icmp eq i32 %t31, 0
  br i1 %t63, label %t64, label %t27

t64:                                               ; preds = %t27
  %t65 = getelementptr i32, ptr %t25, i32 %t4
  %t66 = getelementptr i16, ptr %t24, i32 %t20
  %t67 = getelementptr inbounds i32, ptr %t65, i32 %t7
  %t68 = getelementptr inbounds i16, ptr %t66, i32 %t13
  %t69 = icmp eq i32 %t26, 0
  br i1 %t69, label %t70, label %t22

t70:                                               ; preds = %t64
  br label %t71

t71:                                               ; preds = %t70, %t9
  ret void
}

