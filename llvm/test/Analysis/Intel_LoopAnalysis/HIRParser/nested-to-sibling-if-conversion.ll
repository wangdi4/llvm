; RUN: opt < %s -hir-ssa-deconstruction -xmain-opt-level=3 | opt -analyze -xmain-opt-level=3 -hir-framework -hir-framework-debug=parser | FileCheck %s

; Verify that parser converts the nested ifs into siblings ifs. If one of 'then' or 'else' case of HLIf jumps out of the loop, the other case can be moved after the if.

; CHECK: + DO i1 = 0, %t10 + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %t15.out = &((%t4)[4 * i1]);
; CHECK: |   %t16 = (%t4)[4 * i1];
; CHECK: |   if (&((%t16)[0]) == &((%t1)[0]))
; CHECK: |   {
; CHECK: |      goto t57;
; CHECK: |   }
; CHECK: |   %t19 = &((%t4)[4 * i1 + 1]);
; CHECK: |   %t20 = (%t4)[4 * i1 + 1];
; CHECK: |   if (&((%t20)[0]) == &((%t1)[0]))
; CHECK: |   {
; CHECK: |      goto t59;
; CHECK: |   }
; CHECK: |   %t23 = &((%t4)[4 * i1 + 2]);
; CHECK: |   %t24 = (%t4)[4 * i1 + 2];
; CHECK: |   if (&((%t24)[0]) == &((%t1)[0]))
; CHECK: |   {
; CHECK: |      goto t61;
; CHECK: |   }
; CHECK: |   %t27 = &((%t4)[4 * i1 + 3]);
; CHECK: |   %t28 = (%t4)[4 * i1 + 3];
; CHECK: |   if (&((%t28)[0]) == &((%t1)[0]))
; CHECK: |   {
; CHECK: |      goto t63;
; CHECK: |   }
; CHECK: |   %t31 = &((%t4)[4 * i1 + 4]);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct1 = type { i64, i64, %struct2** }
%struct2 = type <{ i32, [4 x i8] }>
%struct3 = type { %struct1 }

; Function Attrs: uwtable
define void @foo(%struct3* %t0, %struct2* %t1) {
  %t3 = getelementptr inbounds %struct3, %struct3* %t0, i64 0, i32 0, i32 2
  %t4 = load %struct2**, %struct2*** %t3, align 8
  %t5 = getelementptr inbounds %struct3, %struct3* %t0, i64 0, i32 0, i32 0
  %t6 = load i64, i64* %t5, align 8
  %t7 = getelementptr inbounds %struct2*, %struct2** %t4, i64 %t6
  %t8 = ptrtoint %struct2** %t7 to i64
  %t9 = shl nuw i64 %t6, 3
  %t10 = ashr i64 %t9, 5
  %t11 = icmp sgt i64 %t10, 0
  br i1 %t11, label %t12, label %t65

t12:                                     ; preds = %t2
  br label %loop

loop:                                     ; preds = %t30, %t12
  %t14 = phi i64 [ %t32, %t30 ], [ %t10, %t12 ]
  %t15 = phi %struct2** [ %t31, %t30 ], [ %t4, %t12 ]
  %t16 = load %struct2*, %struct2** %t15, align 8
  %t17 = icmp ne %struct2* %t16, %t1
  br i1 %t17, label %t18, label %t57

t18:                                     ; preds = %loop
  %t19 = getelementptr inbounds %struct2*, %struct2** %t15, i64 1
  %t20 = load %struct2*, %struct2** %t19, align 8
  %t21 = icmp eq %struct2* %t20, %t1
  br i1 %t21, label %t59, label %t22

t22:                                     ; preds = %t18
  %t23 = getelementptr inbounds %struct2*, %struct2** %t15, i64 2
  %t24 = load %struct2*, %struct2** %t23, align 8
  %t25 = icmp eq %struct2* %t24, %t1
  br i1 %t25, label %t61, label %t26

t26:                                     ; preds = %t22
  %t27 = getelementptr inbounds %struct2*, %struct2** %t15, i64 3
  %t28 = load %struct2*, %struct2** %t27, align 8
  %t29 = icmp eq %struct2* %t28, %t1
  br i1 %t29, label %t63, label %t30

t30:                                     ; preds = %t26
  %t31 = getelementptr inbounds %struct2*, %struct2** %t15, i64 4
  %t32 = add nsw i64 %t14, -1
  %t33 = icmp sgt i64 %t32, 0
  br i1 %t33, label %loop, label %loopexit

loopexit:                                     ; preds = %t30
  %t35 = phi %struct2** [ %t31, %t30 ]
  br label %t65

t57:                                     ; preds = %loop
  %t58 = phi %struct2** [ %t15, %loop ]
  br label %t65

t59:                                     ; preds = %t18
  %t60 = phi %struct2** [ %t19, %t18 ]
  br label %t65

t61:                                     ; preds = %t22
  %t62 = phi %struct2** [ %t23, %t22 ]
  br label %t65

t63:                                     ; preds = %t26
  %t64 = phi %struct2** [ %t27, %t26 ]
  br label %t65

t65:                                    ; preds = %loopexit, %t57, %t59, %t61, %t63
  ret void
}

