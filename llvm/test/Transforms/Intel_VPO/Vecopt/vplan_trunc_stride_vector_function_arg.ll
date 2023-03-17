; RUN: opt %s -S -passes="vplan-vec" 2>&1 | FileCheck %s
; RUN: opt %s -S -passes="hir-vplan-vec" 2>&1 | FileCheck %s

; Tests checks to see that truncated stride doesn't cause caller/callee
; vector function matching logic to fail. Call should be made to the
; vector function instead of calls getting scalarized.

; CHECK: call noundef <8 x i32> @_ZGVbN8l2__Z3fooi

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @bar(ptr %b) local_unnamed_addr #1 {
omp.inner.for.body.lr.ph:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = shl nuw nsw i64 %indvars.iv, 1
  %2 = trunc i64 %1 to i32
  %call = call noundef i32 @_Z3fooi(i32 noundef %2) #0
  %arrayidx6 = getelementptr inbounds [128 x i32], ptr %b, i64 0, i64 %1
  store i32 %call, ptr %arrayidx6, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  ret i32 0
}

declare i32 @_Z3fooi(i32) #0
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = {  "vector-variants"="_ZGVbN8l2__Z3fooi" }
