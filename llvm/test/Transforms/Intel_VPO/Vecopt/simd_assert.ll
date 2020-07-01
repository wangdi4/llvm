; Test checks that -vplan-simd-assert-default option emits error for not vectorized loop

; Here is original source:
;   extern long A[256];
;   void foo() {
;     #pragma omp simd
;     #pragma nounroll
;     for(volatile int i = 0; i < 2; ++i) A[i] = i;
;   }

; RUN: not opt -S -VPlanDriver -transform-warning -vplan-simd-assert-default < %s 2>&1 | FileCheck %s
; RUN: not opt -S -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -hir-cg -force-hir-cg -transform-warning -vplan-simd-assert-default < %s 2>&1 | FileCheck %s
; CHECK: error: {{.*}}: loop not vectorized:

source_filename = "simd_assert.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = external dso_local local_unnamed_addr global [256 x i64], align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %i = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %i.0.i.0..sroa_cast = bitcast i32* %i to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %.omp.iv.local.07 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ 1, %omp.inner.for.body ]
  store volatile i32 %.omp.iv.local.07, i32* %i, align 4
  %i.0.i.0. = load volatile i32, i32* %i, align 4
  %conv = sext i32 %i.0.i.0. to i64
  %i.0.i.0.3 = load volatile i32, i32* %i, align 4
  %idxprom = sext i32 %i.0.i.0.3 to i64
  %arrayidx = getelementptr inbounds [256 x i64], [256 x i64]* @A, i64 0, i64 %idxprom
  store i64 %conv, i64* %arrayidx, align 8
  %cmp = icmp eq i32 %.omp.iv.local.07, 0
  br i1 %cmp, label %omp.inner.for.body, label %DIR.OMP.END.SIMD.4, !llvm.loop !1

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.unroll.disable"}

