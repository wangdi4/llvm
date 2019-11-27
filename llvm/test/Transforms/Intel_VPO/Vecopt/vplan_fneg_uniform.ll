; Check that VPlan vectorizes uniform fneg instruction properly
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen=false -vplan-force-vf=4 < %s 2>&1 | FileCheck %s -check-prefixes=CHECK-IR,CHECK-LLVM-IR
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -vplan-force-vf=4 < %s 2>&1 | FileCheck %s -check-prefixes=CHECK-IR,CHECK-VPVALUE-IR
; RUN: opt -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -enable-vp-value-codegen-hir=false -vplan-force-vf=4 < %s | FileCheck %s -check-prefix=CHECK-HIR
; RUN: opt -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -enable-vp-value-codegen-hir -vplan-force-vf=4 < %s | FileCheck %s -check-prefix=CHECK-HIR

; CHECK-IR:         vector.body:
; CHECK-LLVM-IR:      fneg <4 x float>
; CHECK-VPVALUE-IR:   fneg float
; CHECK-IR:         omp.inner.for.body:
; CHECK-IR:           fneg float

; CHECK-HIR: fneg <4 x float>
; CHECK-HIR: fneg float

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @f(float* nocapture readonly %a, float* nocapture %b, float %m, i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count20 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %add6 = fadd float %1, %m
  %arrayidx8 = getelementptr inbounds float, float* %b, i64 %indvars.iv
  store float %add6, float* %arrayidx8, align 4
  %uniform.fneg = fneg float %m
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count20
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
