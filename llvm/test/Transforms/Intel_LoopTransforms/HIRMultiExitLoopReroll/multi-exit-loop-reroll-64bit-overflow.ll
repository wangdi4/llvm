; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,print<hir>" -xmain-opt-level=3 2>&1 | FileCheck %s

; Verify that we bailt out of multi-exit loop reroll pass if the rerolled loop's trip count overflows.

; HIR-
; + DO i1 = 0, 4611686018427387903, 1   <DO_MULTI_EXIT_LOOP>
; |   if ((%t4)[4 * i1] == &((%t1)[0]))
; |   {
; |      %t15.out = &((%t4)[4 * i1]);
; |      goto t57;
; |   }
; |   if ((%t4)[4 * i1 + 1] == &((%t1)[0]))
; |   {
; |      %t19 = &((%t4)[4 * i1 + 1]);
; |      goto t59;
; |   }
; |   if ((%t4)[4 * i1 + 2] == &((%t1)[0]))
; |   {
; |      %t23 = &((%t4)[4 * i1 + 2]);
; |      goto t61;
; |   }
; |   if ((%t4)[4 * i1 + 3] == &((%t1)[0]))
; |   {
; |      %t27 = &((%t4)[4 * i1 + 3]);
; |      goto t63;
; |   }
; + END LOOP


; CHECK: + DO i1 = 0, 4611686018427387903, 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct1 = type { i64, i64, ptr }
%struct2 = type <{ i32, [4 x i8] }>
%struct3 = type { %struct1 }

; Function Attrs: uwtable
define void @foo(ptr %t0, ptr %t1) {
  %t3 = getelementptr inbounds %struct3, ptr %t0, i64 0, i32 0, i32 2
  %t4 = load ptr, ptr %t3, align 8
  %t5 = getelementptr inbounds %struct3, ptr %t0, i64 0, i32 0, i32 0
  %t6 = load i64, ptr %t5, align 8
  %t7 = getelementptr inbounds ptr, ptr %t4, i64 %t6
  %t8 = ptrtoint ptr %t7 to i64
  %t9 = shl nuw i64 %t6, 3
  br label %t12

t12:                                     ; preds = %t2
  br label %loop

loop:                                     ; preds = %t30, %t12
  %t14 = phi i64 [ %t32, %t30 ], [ 4611686018427387904, %t12 ]
  %t15 = phi ptr [ %t31, %t30 ], [ %t4, %t12 ]
  %t16 = load ptr, ptr %t15, align 8
  %t17 = icmp ne ptr %t16, %t1
  br i1 %t17, label %t18, label %t57

t18:                                     ; preds = %loop
  %t19 = getelementptr inbounds ptr, ptr %t15, i64 1
  %t20 = load ptr, ptr %t19, align 8
  %t21 = icmp eq ptr %t20, %t1
  br i1 %t21, label %t59, label %t22

t22:                                     ; preds = %t18
  %t23 = getelementptr inbounds ptr, ptr %t15, i64 2
  %t24 = load ptr, ptr %t23, align 8
  %t25 = icmp eq ptr %t24, %t1
  br i1 %t25, label %t61, label %t26

t26:                                     ; preds = %t22
  %t27 = getelementptr inbounds ptr, ptr %t15, i64 3
  %t28 = load ptr, ptr %t27, align 8
  %t29 = icmp eq ptr %t28, %t1
  br i1 %t29, label %t63, label %t30

t30:                                     ; preds = %t26
  %t31 = getelementptr inbounds ptr, ptr %t15, i64 4
  %t32 = add nsw i64 %t14, -1
  %t33 = icmp sgt i64 %t32, 0
  br i1 %t33, label %loop, label %loopexit

loopexit:                                     ; preds = %t30
  %t35 = phi ptr [ %t31, %t30 ]
  br label %t65

t57:                                     ; preds = %loop
  %t58 = phi ptr [ %t15, %loop ]
  br label %t65

t59:                                     ; preds = %t18
  %t60 = phi ptr [ %t19, %t18 ]
  br label %t65

t61:                                     ; preds = %t22
  %t62 = phi ptr [ %t23, %t22 ]
  br label %t65

t63:                                     ; preds = %t26
  %t64 = phi ptr [ %t27, %t26 ]
  br label %t65

t65:                                    ; preds = %loopexit, %t57, %t59, %t61, %t63
  %t66 = phi ptr [ %t64, %t63 ], [ %t62, %t61 ], [ %t60, %t59 ], [ %t58, %t57 ], [ %t35, %loopexit ]
  br label %t67

t67:
  ret void
}

