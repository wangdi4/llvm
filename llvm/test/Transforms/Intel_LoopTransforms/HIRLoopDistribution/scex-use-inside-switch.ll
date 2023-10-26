; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output -hir-loop-distribute-skip-vectorization-profitability-check=true %s 2>&1 | FileCheck %s

; Verify that we don't compfail on figuring out where to put the definition of
; %t43 in the second chunk. We decided to split the definition and use of %t43
; across chunks. We also decided to 'reload/recompute' %t43 instead of scalar-
; expanding it. However, the analysis didn't expect that the use of %t43 can be
; inside a switch and failed.

; Incoming HIR-
; + DO i1 = 0, 4294967294, 1   <DO_LOOP>
; |   %t9 = 0.000000e+00  *  (null)[0];
; |   %t14 = (null)[0]  *  (null)[0];
; |   %t17 = (null)[0]  *  (null)[0];
; |   %t19 = 0.000000e+00  *  (null)[0];
; |   %t24 = (null)[0]  *  (null)[0];
; |   %t27 = (null)[0]  *  (null)[0];
; |   %t29 = 0.000000e+00  *  (null)[0];
; |   %t33 = (%t0)[0]  *  0.000000e+00;
; |   %t34 = 0.000000e+00  *  (null)[0];
; |   if (%t2 != 0)
; |   {
; |      (null)[0] = (null)[0];
; |   }
; |   else
; |   {
; |      %t43 = (%t0)[0];
; |      %t44 = 0.000000e+00  *  %t43;
; |      switch(%t1)
; |      {
; |      case 0:
; |         %t37 = 0.000000e+00  *  %t43;
; |         %t38 = 0.000000e+00  +  (null)[0];
; |         (null)[0] = 0.000000e+00;
; |         (null)[0] = (null)[0];
; |         break;
; |      default:
; |         break;
; |      }
; |   }
; + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 4294967294, 1   <DO_LOOP>
; CHECK: |   %t33 = (%t0)[0]  *  0.000000e+00;
; CHECK: |   if (%t2 == 0)
; CHECK: |   {
; CHECK: |      %t43 = (%t0)[0];
; CHECK: |      %t44 = 0.000000e+00  *  %t43;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, 4294967294, 1   <DO_LOOP>
; CHECK: |   %t9 = 0.000000e+00  *  (null)[0];
; CHECK: |   %t14 = (null)[0]  *  (null)[0];
; CHECK: |   %t17 = (null)[0]  *  (null)[0];
; CHECK: |   %t19 = 0.000000e+00  *  (null)[0];
; CHECK: |   %t24 = (null)[0]  *  (null)[0];
; CHECK: |   %t27 = (null)[0]  *  (null)[0];
; CHECK: |   %t29 = 0.000000e+00  *  (null)[0];
; CHECK: |   %t34 = 0.000000e+00  *  (null)[0];
; CHECK: |   if (%t2 != 0)
; CHECK: |   {
; CHECK: |      (null)[0] = (null)[0];
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %t43 = (%t0)[0];
; CHECK: |      switch(%t1)
; CHECK: |      {
; CHECK: |      case 0:
; CHECK: |         %t37 = 0.000000e+00  *  %t43;
; CHECK: |         %t38 = 0.000000e+00  +  (null)[0];
; CHECK: |         (null)[0] = 0.000000e+00;
; CHECK: |         (null)[0] = (null)[0];
; CHECK: |         break;
; CHECK: |      default:
; CHECK: |         break;
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define fastcc void @brthso_(ptr %t0, i32 %t1, i1 %t2, i1 %t3) {
entry:
  br label %loop

loop:                                                ; preds = %latch, %t4
  %t6 = phi i64 [ 1, %entry ], [ %t46, %latch ]
  %t8 = load double, ptr null, align 1
  %t9 = fmul double 0.000000e+00, %t8
  %t12 = load double, ptr null, align 1
  %t13 = load double, ptr null, align 1
  %t14 = fmul double %t13, %t12
  %t15 = load double, ptr null, align 1
  %t16 = load double, ptr null, align 1
  %t17 = fmul double %t16, %t15
  %t18 = load double, ptr null, align 1
  %t19 = fmul double 0.000000e+00, %t18
  %t22 = load double, ptr null, align 1
  %t23 = load double, ptr null, align 1
  %t24 = fmul double %t23, %t22
  %t25 = load double, ptr null, align 1
  %t26 = load double, ptr null, align 1
  %t27 = fmul double %t26, %t25
  %t28 = load double, ptr null, align 1
  %t29 = fmul double 0.000000e+00, %t28
  br label %t30

t30:                                               ; preds = %t21, %t20
  %t31 = load double, ptr %t0, align 8
  %t32 = load double, ptr null, align 8
  %t33 = fmul double %t31, 0.000000e+00
  %t34 = fmul double 0.000000e+00, %t32
  br i1 %t2, label %t40, label %t42

t35:                                               ; preds = %t42
  %t36 = load double, ptr null, align 16
  %t37 = fmul double 0.000000e+00, %t43
  %t38 = fadd double 0.000000e+00, %t36
  store double 0.000000e+00, ptr null, align 16
  %t39 = load double, ptr null, align 16
  store double %t39, ptr null, align 16
  br label %latch

t40:                                               ; preds = %t30
  %t41 = load double, ptr null, align 1
  store double %t41, ptr null, align 1
  br label %latch

t42:                                               ; preds = %t30
  %t43 = load double, ptr %t0, align 1
  %t44 = fmul double 0.000000e+00, %t43
  switch i32 %t1, label %latch [
    i32 0, label %t35
  ]

latch:                                               ; preds = %t42, %t40, %t35
  %t46 = add i64 %t6, 1
  %t47 = trunc i64 %t46 to i32
  %t48 = icmp eq i32 0, %t47
  br i1 %t48, label %exit, label %loop

exit:                                               ; preds = %latch
  ret void
}
