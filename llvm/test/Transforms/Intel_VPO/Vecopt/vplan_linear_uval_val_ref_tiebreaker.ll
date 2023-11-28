; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check parameter scoring that favors uval variant over val and ref variants.

; CHECK: call noundef <8 x i64> @_ZGVbN8U__Z3fooRl

define i32 @main() {
DIR.OMP.SIMD.123:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.123
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %i.linear.iv, i64 0, i64 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.016 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add1, %omp.inner.for.body ]
  store i64 %.omp.iv.local.016, ptr %i.linear.iv, align 8
  %call = call noundef i64 @_Z3fooRl(ptr noundef nonnull align 8 dereferenceable(8) %i.linear.iv)
  %1 = load i64, ptr %i.linear.iv, align 8
  %arrayidx = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %1
  store i64 %call, ptr %arrayidx, align 8
  %add1 = add nuw nsw i64 %.omp.iv.local.016, 1
  %exitcond.not = icmp eq i64 %add1, 128
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.414, label %omp.inner.for.body

DIR.OMP.END.SIMD.414:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.414
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare i64 @_Z3fooRl(ptr noundef nonnull align 8 dereferenceable(8)) #0

attributes #0 = { "vector-variants"="_ZGVbN8L__Z3fooRl,_ZGVbN8U__Z3fooRl,_ZGVbN8R8__Z3fooRl" }
