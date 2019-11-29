; Test to check VPlan's VPValue-based vector codegen for simple in-register integer max reduction.

; Incoming HIR into vectorizer
; <0>    BEGIN REGION { }
; <2>          %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.MAX(&((%s.red)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <3>          (%s.red)[0] = -2147483648;
; <6>          %.23 = -2147483648;
; <26>
; <26>          + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP> <simd>
; <11>          |   %1 = (%ptr)[i1];
; <13>          |   %.23 = (%.23 > %1) ? %.23 : %1;
; <26>          + END LOOP
; <26>
; <21>          (%s.red)[0] = %.23;
; <23>          %max = (%.23 > 10) ? %.23 : 10;
; <24>          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>    END REGION

; Fully VPValue-based HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR
; Mixed HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR

; CHECK-HIR: Function: foo_int
; CHECK-HIR: if (0 <u 4 * [[UB:%.*]])
; CHECK-HIR: [[RED_VAR:%.*]] = [[RED_INIT:%.*]];
; CHECK-HIR: DO i1 = 0, 4 * [[UB]] + -1, 4   <DO_LOOP>
; CHECK-HIR: [[VEC_LD:%.*]] = (<4 x i32>*)(%ptr)
; CHECK-HIR: [[RED_VAR]] = ([[RED_VAR]] > [[VEC_LD]]) ? [[RED_VAR]] : [[VEC_LD]];
; CHECK-HIR: END LOOP
; CHECK-HIR: [[RED_INIT]] = @llvm.experimental.vector.reduce.smax.v4i32([[RED_VAR]]);


; Fully VPValue-based LLVM-IR codegen
; RUN: opt -vpo-cfg-restructuring -VPlanDriver -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen -S < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVMIR

; CHECK-LLVMIR-LABEL: @foo_int
; CHECK-LLVMIR-LABEL: vector.body:
; CHECK-LLVMIR: [[RED_PHI:%.*]] = phi <4 x i32> [ <i32 -2147483648, i32 -2147483648, i32 -2147483648, i32 -2147483648>, %vector.ph ], [ [[RED_ADD:%.*]], %vector.body ]
; CHECK-LLVMIR: [[RED_CMP:%.*]] = icmp sgt <4 x i32> [[RED_PHI]], [[VEC_LD:%.*]]
; CHECK-LLVMIR: [[RED_SELECT:%.*]] = select <4 x i1> [[RED_CMP]], <4 x i32> [[RED_PHI]], <4 x i32> [[VEC_LD]]
; CHECK-LLVMIR-LABEL: VPlannedBB:
; CHECK-LLVMIR: [[RED_LVC:%.*]] = call i32 @llvm.experimental.vector.reduce.smax.v4i32(<4 x i32> [[RED_SELECT]])
; CHECK-LLVMIR-LABEL: scalar.ph:
; CHECK-LLVMIR: [[MERGE_RED_PHI:%.*]] = phi i32 [ -2147483648, %DIR.OMP.SIMD.2 ], [ [[RED_LVC]], %middle.block ]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo_int(i32* nocapture readonly %ptr, i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %s.red = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX"(i32* %s.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  store i32 -2147483648, i32* %s.red, align 4
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %.23 = phi i32 [ -2147483648, %DIR.OMP.SIMD.1 ], [ %., %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp6 = icmp sgt i32 %.23, %1
  %. = select i1 %cmp6, i32 %.23, i32 %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit_crit_edge.split.split: ; preds = %omp.inner.for.body
  %..lcssa = phi i32 [ %., %omp.inner.for.body ]
  store i32 %..lcssa, i32* %s.red, align 4, !tbaa !2
  %2 = icmp sgt i32 %..lcssa, 10
  %max = select i1 %2, i32 %..lcssa, i32 10
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, %entry
  %s.1 = phi i32 [ %max, %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split ], [ 10, %entry ]
  ret i32 %s.1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
