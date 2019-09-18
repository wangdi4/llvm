; This test checks that the operands of the 'select' instructions are correctly widened.

;RUN: opt -VPlanDriver -vplan-force-vf=2 -S %s | FileCheck %s

;CHECK-LABEL: @getElement
;CHECK:  [[REP_VAL1:%.*]] = shufflevector <3 x float> %s1, <3 x float> undef, <6 x i32> <i32 0, i32 1, i32 2, i32 0, i32 1, i32 2>
;CHECK:  [[REP_VAL2:%.*]] = shufflevector <3 x float> %s2, <3 x float> undef, <6 x i32> <i32 0, i32 1, i32 2, i32 0, i32 1, i32 2>
;CHECK:  [[WIDE_COND:%.*]] = shufflevector <2 x i1> %1, <2 x i1> undef, <6 x i32> <i32 0, i32 0, i32 0, i32 1, i32 1, i32 1>
;CHECK:  [[WIDE_SELECT:%.*]] = select <6 x i1> [[WIDE_COND]], <6 x float> [[REP_VAL1]], <6 x float> [[REP_VAL2]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local float @getElement(<3 x float> %s1, <3 x float> %s2, i32 %idx) {
omp.inner.for.body.lr.ph:
  %t.priv = alloca <3 x float>, align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(<3 x float>* %t.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast <3 x float>* %t.priv to i8*
  %rem = and i32 %idx, 3
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %.omp.iv.local.0 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add3, %omp.inner.for.body ]
  %retVal.015 = phi float [ 0.000000e+00, %DIR.OMP.SIMD.2 ], [ %add2, %omp.inner.for.body ]
  %cmp1 = icmp ugt i32 %.omp.iv.local.0, %idx
  %2 = select i1 %cmp1, <3 x float> %s1, <3 x float> %s2
  %vecext = extractelement <3 x float> %2, i32 %rem
  %add2 = fadd float %retVal.015, %vecext
  %add3 = add nuw nsw i32 %.omp.iv.local.0, 1
  %exitcond = icmp eq i32 %add3, 4096
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  %add2.lcssa = phi float [ %add2, %omp.inner.for.body ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.419

DIR.OMP.END.SIMD.419:                             ; preds = %DIR.OMP.END.SIMD.3
  ret float %add2.lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)
