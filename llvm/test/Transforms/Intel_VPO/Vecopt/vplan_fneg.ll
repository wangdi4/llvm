; Check that VPlan vectorizes and revectorizes fneg instruction properly
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen=false -vplan-force-vf=4 < %s | FileCheck %s --check-prefixes=CHECK-IR,CHECK-BOTH
; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -vplan-force-vf=4 < %s | FileCheck %s --check-prefixes=CHECK-IR,CHECK-BOTH
; RUN: opt -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -enable-vp-value-codegen-hir=false -vplan-force-vf=4 < %s | FileCheck %s --check-prefixes=CHECK-HIR,CHECK-BOTH
; RUN: opt -S -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -enable-vp-value-codegen-hir -vplan-force-vf=4 < %s | FileCheck %s --check-prefixes=CHECK-HIR,CHECK-BOTH

; CHECK-BOTH-LABEL: @f
; CHECK-BOTH: fneg <4 x float>

; CHECK-IR-LABEL: @f_revectorize
; CHECK-IR: fneg <8 x float>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @f(float* nocapture readonly %a, float* nocapture %b, i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count19 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %fneg = fneg float %1
  %arrayidx7 = getelementptr inbounds float, float* %b, i64 %indvars.iv
  store float %fneg, float* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count19
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @f_revectorize(<2 x float>* nocapture readonly %a, <2 x float>* nocapture %b, i32 %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count19 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds <2 x float>, <2 x float>* %a, i64 %indvars.iv
  %1 = load <2 x float>, <2 x float>* %arrayidx, align 8
  %fneg = fneg <2 x float> %1
  %arrayidx7 = getelementptr inbounds <2 x float>, <2 x float>* %b, i64 %indvars.iv
  store <2 x float> %fneg, <2 x float>* %arrayidx7, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count19
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
