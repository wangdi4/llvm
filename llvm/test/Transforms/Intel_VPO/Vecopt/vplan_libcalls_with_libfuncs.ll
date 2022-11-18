; Verify that VPlan vectorizes library calls which have valid LibFuncs
; using vector version from SVML.

; RUN: opt < %s -vplan-vec -vplan-force-vf=2 -vector-library=SVML -S | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(double %darg, float %farg) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]

  %call1 = call afn double @cdfnorm(double %darg) #2
  ; CHECK: __svml_cdfnorm2
  %call2 = call afn float @cdfnormf(float %farg) #2
  ; CHECK: __svml_cdfnormf2

  %call3 = call afn double @cdfnorminv(double %darg) #2
  ; CHECK: __svml_cdfnorminv2
  %call4 = call afn float @cdfnorminvf(float %farg) #2
  ; CHECK: __svml_cdfnorminvf2

  %call5 = call afn double @erf(double %darg) #2
  ; CHECK: __svml_erf2
  %call6 = call afn float @erff(float %farg) #2
  ; CHECK: __svml_erff2

  %call7 = call afn double @erfc(double %darg) #2
  ; CHECK: __svml_erfc2
  %call8 = call afn float @erfcf(float %farg) #2
  ; CHECK: __svml_erfcf2

  %call9 = call afn double @erfcinv(double %darg) #2
  ; CHECK: __svml_erfcinv2
  %call10 = call afn float @erfcinvf(float %farg) #2
  ; CHECK: __svml_erfcinvf2

  %call11 = call afn double @erfinv(double %darg) #2
  ; CHECK: __svml_erfinv2
  %call12 = call afn float @erfinvf(float %farg) #2
  ; CHECK: __svml_erfinvf2

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local double @cdfnorm(double) local_unnamed_addr #3
declare dso_local float @cdfnormf(float) local_unnamed_addr #3
declare dso_local double @cdfnorminv(double) local_unnamed_addr #3
declare dso_local float @cdfnorminvf(float) local_unnamed_addr #3
declare dso_local double @erf(double) local_unnamed_addr #3
declare dso_local float @erff(float) local_unnamed_addr #3
declare dso_local double @erfc(double) local_unnamed_addr #3
declare dso_local float @erfcf(float) local_unnamed_addr #3
declare dso_local double @erfcinv(double) local_unnamed_addr #3
declare dso_local float @erfcinvf(float) local_unnamed_addr #3
declare dso_local double @erfinv(double) local_unnamed_addr #3
declare dso_local float @erfinvf(float) local_unnamed_addr #3

attributes #2 = { nounwind readnone }
