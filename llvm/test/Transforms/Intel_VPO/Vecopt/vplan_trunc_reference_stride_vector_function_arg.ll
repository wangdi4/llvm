; RUN: opt %s -S -passes="vplan-vec" 2>&1 | FileCheck %s
; RUN: opt %s -S -passes="hir-vplan-vec" 2>&1 | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

; CHECK: call noundef <8 x i32> @"_ZGVbN8U2_?foo@@YAJAEAJ@Z"

define i32 @bar(ptr %a) {
omp.inner.for.body.lr.ph:
  %i2.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i2.linear.iv, i32 0, i32 1, i32 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %2 = trunc i64 %1 to i32
  store i32 %2, ptr %i2.linear.iv, align 4
  %call = call noundef i32 @"?foo@@YAJAEAJ@Z"(ptr noundef nonnull align 4 dereferenceable(4) %i2.linear.iv)
  %arrayidx4 = getelementptr inbounds [128 x i32], ptr %a, i64 0, i64 %1
  store i32 %call, ptr %arrayidx4, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  ret i32 0
}

declare i32 @"?foo@@YAJAEAJ@Z"(ptr) #0
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbN8U2_?foo@@YAJAEAJ@Z" }
