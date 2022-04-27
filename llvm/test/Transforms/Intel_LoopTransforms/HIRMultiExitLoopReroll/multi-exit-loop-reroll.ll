; RUN: opt < %s -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-multi-exit-loop-reroll -print-after=hir-multi-exit-loop-reroll 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,print<hir>" -xmain-opt-level=3 2>&1 | FileCheck %s

; RUN: opt < %s -opaque-pointers -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-multi-exit-loop-reroll -print-after=hir-multi-exit-loop-reroll 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,print<hir>" -xmain-opt-level=3 2>&1 | FileCheck %s

; Verify that multi-exit loop reroll pass triggers on this loop.

; HIR-
; + DO i1 = 0, %t10 + -1, 1   <DO_MULTI_EXIT_LOOP>
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


; CHECK: + DO i1 = 0, 4 * %t10 + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   if ((%t4)[i1] == &((%t1)[0]))
; CHECK: |   {
; CHECK: |      %t15.out = &((%t4)[i1]);
; CHECK: |      goto t57;
; CHECK: |   }
; CHECK: + END LOOP


; Verify that we emit an opt report remark.

; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-multi-exit-loop-reroll -hir-cg -intel-opt-report=low -intel-ir-optreport-emitter -simplifycfg -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-multi-exit-loop-reroll,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #25264: Loop rerolled by 4
; OPTREPORT: LOOP END


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
  br i1 %t11, label %t12, label %t67

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
  %t66 = phi %struct2** [ %t64, %t63 ], [ %t62, %t61 ], [ %t60, %t59 ], [ %t58, %t57 ], [ %t35, %loopexit ]
  br label %t67

t67:
  ret void
}

