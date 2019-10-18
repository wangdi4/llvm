; Test to check VPlan's VPValue-based vector codegen for simple in-register integer add reduction.

; Incoming HIR into vectorizer
; <0>     BEGIN REGION { }
; <2>           %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%s.red)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <3>           (%s.red)[0] = 0;
; <6>           %add618 = 0;
; <23>
; <23>          + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP> <simd>
; <10>          |   %1 = (%ptr)[i1];
; <11>          |   %add618 = %1  +  %add618;
; <23>          + END LOOP
; <23>
; <19>          (%s.red)[0] = %add618;
; <20>          %2 = %add618  +  10;
; <21>          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>     END REGION

; Fully VPValue-based HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen-hir -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR
; Mixed HIR codegen
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -VPlanDriverHIR -vplan-force-vf=4 -vplan-use-entity-instr -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR

; CHECK-HIR-LABEL: Function: foo_int
; CHECK-HIR: if (0 <u 4 * [[UB:%.*]])
; CHECK-HIR: [[RED_VAR:%.*]] = 0;
; CHECK-HIR: [[RED_VAR]] = insertelement [[RED_VAR]],  [[RED_INIT:%.*]],  0;
; CHECK-HIR: DO i1 = 0, 4 * [[UB]] + -1, 4   <DO_LOOP>
; CHECK-HIR: [[VEC_LD:%.*]] = (<4 x i32>*)(%ptr)
; CHECK-HIR: [[RED_VAR]] = [[VEC_LD]]  +  [[RED_VAR]];
; CHECK-HIR: END LOOP
; CHECK-HIR: [[RED_INIT]] = @llvm.experimental.vector.reduce.add.v4i32([[RED_VAR]]);


; Fully VPValue-based LLVM-IR codegen
; RUN: opt -vpo-cfg-restructuring -VPlanDriver -vplan-force-vf=4 -vplan-use-entity-instr -enable-vp-value-codegen -S < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVMIR

; CHECK-LLVMIR-LABEL: @foo_int
; CHECK-LLVMIR-LABEL: vector.body:
; CHECK-LLVMIR: [[RED_PHI:%.*]] = phi <4 x i32> [ zeroinitializer, %vector.ph ], [ [[RED_ADD:%.*]], %vector.body ]
; CHECK-LLVMIR: [[RED_ADD]] = add nsw <4 x i32> {{%.*}}, [[RED_PHI]]
; CHECK-LLVMIR-LABEL: VPlannedBB:
; CHECK-LLVMIR: [[RED_LVC:%.*]] = call i32 @llvm.experimental.vector.reduce.add.v4i32(<4 x i32> [[RED_ADD]])
; CHECK-LLVMIR-LABEL: scalar.ph:
; CHECK-LLVMIR: [[MERGE_RED_PHI:%.*]] = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ 0, %min.iters.checked ], [ [[RED_LVC]], %middle.block ]


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo_int(i32* nocapture readonly %ptr, i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %s.red = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i32* %s.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  store i32 0, i32* %s.red, align 4
  %wide.trip.count = sext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %add618 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add6, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ptr, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add6 = add nsw i32 %1, %add618
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, label %omp.inner.for.body

omp.inner.for.cond.omp.loop.exit_crit_edge.split.split: ; preds = %omp.inner.for.body
  %add6.lcssa = phi i32 [ %add6, %omp.inner.for.body ]
  store i32 %add6.lcssa, i32* %s.red, align 4, !tbaa !2
  %2 = add i32 %add6.lcssa, 10
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split, %entry
  %s.1 = phi i32 [ %2, %omp.inner.for.cond.omp.loop.exit_crit_edge.split.split ], [ 10, %entry ]
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
