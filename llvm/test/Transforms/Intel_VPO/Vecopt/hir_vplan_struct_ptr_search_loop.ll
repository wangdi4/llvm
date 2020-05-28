; Test to verify that structptreq search loop idiom is recognized on this loop, and correct vector code with peel loop and alignment is generated.

; Check if search loop was recognized
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -hir-multi-exit-loop-reroll -hir-vec-dir-insert -VPlanDriverHIR -debug-only=vplan-idioms < %s 2>&1 | FileCheck --check-prefix=WAS-RECOGNIZED-CHECK %s

; Check final vectorized codegen
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:     -hir-multi-exit-loop-reroll -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output < %s 2>&1 | FileCheck --check-prefix=CG-CHECK %s

; REQUIRES: asserts

; HIR before VPlan-
; + DO i1 = 0, 4 * %t10 + -1, 1   <DO_MULTI_EXIT_LOOP>
; |   if ((%t4)[i1] == &((%t1)[0]))
; |   {
; |      %t15.out = &((%t4)[i1]);
; |      goto t57;
; |   }
; + END LOOP

; WAS-RECOGNIZED-CHECK: StructPtrEq loop has PeelArray:(%t4)[i1]
; WAS-RECOGNIZED-CHECK: Search loop was recognized.

; -----------------------------------------------------------------------------
; After VPlan
; CG-CHECK:          BEGIN REGION { modified }
; CG-CHECK-NEXT:           %arr.base.cast = ptrtoint.%struct2**.i64(&((%t4)[0]));
; CG-CHECK-NEXT:           %alignment = %arr.base.cast  &  31;
; CG-CHECK-NEXT:           %peel.factor = 32  -  %alignment;
; CG-CHECK-NEXT:           %peel.factor1 = %peel.factor  >>  3;
; CG-CHECK-NEXT:           %peel.factor1 = (4 * %t10 <=u %peel.factor1) ? 4 * %t10 : %peel.factor1;
; CG-CHECK-NEXT:           if (%peel.factor1 != 0)
; CG-CHECK-NEXT:           {
; CG-CHECK-NEXT:              + DO i1 = 0, %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>
; CG-CHECK-NEXT:              |   if ((%t4)[i1] == &((%t1)[0]))
; CG-CHECK-NEXT:              |   {
; CG-CHECK-NEXT:              |      %t15.out = &((%t4)[i1]);
; CG-CHECK-NEXT:              |      goto t57;
; CG-CHECK-NEXT:              |   }
; CG-CHECK-NEXT:              + END LOOP
; CG-CHECK-NEXT:           }
; CG-CHECK-NEXT:           if (%peel.factor1 <u 4 * %t10)
; CG-CHECK-NEXT:           {
; CG-CHECK-NEXT:              %tgu = (4 * %t10 + -1 * %peel.factor1)/u4;
; CG-CHECK-NEXT:              if (0 <u 4 * %tgu)
; CG-CHECK-NEXT:              {
; CG-CHECK-NEXT:                 + DO i1 = 0, 4 * %tgu + -1, 4   <DO_MULTI_EXIT_LOOP> <nounroll> <novectorize>
; CG-CHECK-NEXT:                 |   %wide.cmp. = (<4 x %struct2*>*)(%t4)[i1 + %peel.factor1] == &((<4 x %struct2*>)(%t1)[0]);
; CG-CHECK-NEXT:                 |   %intmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CG-CHECK-NEXT:                 |   if (%intmask != 0)
; CG-CHECK-NEXT:                 |   {
; CG-CHECK-NEXT:                 |      %.vec = %wide.cmp.  ^  -1;
; CG-CHECK-NEXT:                 |      %bsfintmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CG-CHECK-NEXT:                 |      %bsf = @llvm.cttz.i4(%bsfintmask,  1);
; CG-CHECK-NEXT:                 |      %cast = zext.i4.i64(%bsf);
; CG-CHECK-NEXT:                 |      %t15.out = &((%t4)[i1 + %peel.factor1 + %cast]);
; CG-CHECK-NEXT:                 |      goto t57;
; CG-CHECK-NEXT:                 |   }
; CG-CHECK-NEXT:                 + END LOOP
; CG-CHECK-NEXT:              }
; CG-CHECK:                   + DO i1 = 4 * %tgu, 4 * %t10 + -1 * %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CG-CHECK-NEXT:              |   if ((%t4)[i1 + %peel.factor1] == &((%t1)[0]))
; CG-CHECK-NEXT:              |   {
; CG-CHECK-NEXT:              |      %t15.out = &((%t4)[i1 + %peel.factor1]);
; CG-CHECK-NEXT:              |      goto t57;
; CG-CHECK-NEXT:              |   }
; CG-CHECK-NEXT:              + END LOOP
; CG-CHECK-NEXT:           }
; CG-CHECK-NEXT:           %t31 = &((%t4)[4 * %t10]);
; CG-CHECK:          END REGION


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

