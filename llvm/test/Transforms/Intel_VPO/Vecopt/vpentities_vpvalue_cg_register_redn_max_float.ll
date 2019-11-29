; Test to check VPlan's VPValue-based vector codegen for simple in-register float max reduction.

; Fully VPValue-based HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR
; Mixed HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR

; CHECK-HIR: Function: foo_float
; CHECK-HIR: if (0 <u 4 * [[UB:%.*]])
; CHECK-HIR: [[RED_VAR:%.*]] = [[RED_INIT:%.*]];
; CHECK-HIR: DO i1 = 0, 4 * [[UB]] + -1, 4   <DO_LOOP>
; CHECK-HIR: [[VEC_LD:%.*]] = (<4 x float>*)(%ptr)
; CHECK-HIR: [[RED_VAR]] = ([[RED_VAR]] > [[VEC_LD]]) ? [[RED_VAR]] : [[VEC_LD]];
; CHECK-HIR: END LOOP
; CHECK-HIR: [[RED_INIT]] = @llvm.experimental.vector.reduce.fmax.v4f32([[RED_VAR]]);


; Fully VPValue-based LLVM-IR codegen
; RUN: opt -vpo-cfg-restructuring -VPlanDriver -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen -S < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVMIR

; CHECK-LLVMIR-LABEL: @foo_float
; CHECK-LLVMIR-LABEL: vector.body:
; CHECK-LLVMIR: [[RED_PHI:%.*]] = phi <4 x float> [ <float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000, float 0xFFF0000000000000>, %vector.ph ], [ [[RED_ADD:%.*]], %vector.body ]
; CHECK-LLVMIR: [[RED_CMP:%.*]] = fcmp ogt <4 x float> [[RED_PHI]], [[VEC_LD:%.*]]
; CHECK-LLVMIR: [[RED_SELECT:%.*]] = select <4 x i1> [[RED_CMP]], <4 x float> [[RED_PHI]], <4 x float> [[VEC_LD]]
; CHECK-LLVMIR-LABEL: VPlannedBB:
; CHECK-LLVMIR: [[RED_LVC:%.*]] = call float @llvm.experimental.vector.reduce.fmax.v4f32(<4 x float> [[RED_SELECT]])
; CHECK-LLVMIR-LABEL: scalar.ph:
; CHECK-LLVMIR: [[MERGE_RED_PHI:%.*]] = phi float [ 0xFFF0000000000000, %DIR.OMP.SIMD.2 ], [ [[RED_LVC]], %middle.block ]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local float @foo_float(float* nocapture readonly %ptr, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %s.red = alloca float, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX"(float* %s.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  store float 0xFFF0000000000000, float* %s.red, align 4
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %.23 = phi float [ 0xFFF0000000000000, %DIR.OMP.SIMD.1 ], [ %., %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %ptr, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4, !tbaa !2
  %cmp6 = fcmp ogt float %.23, %1
  %. = select i1 %cmp6, float %.23, float %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit_crit_edge.split.split: ; preds = %omp.inner.for.body
  %..lcssa = phi float [ %., %omp.inner.for.body ]
  store float %..lcssa, float* %s.red, align 4, !tbaa !2
  %isOGT = fcmp olt float %..lcssa, 1.000000e+01
  %max = select i1 %isOGT, float 1.000000e+01, float %..lcssa
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, %entry
  %s.1 = phi float [ %max, %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split ], [ 1.000000e+01, %entry ]
  ret float %s.1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { norecurse nounwind uwtable "no-nans-fp-math"="true" }
attributes #1 = { nounwind }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
