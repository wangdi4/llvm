; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/callee simd function matching is successful for a variant
; without parameters.

; CHECK-LABEL: vector.body:
; CHECK: call <2 x i64> @_ZGVbN2__Z3foov

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

@b = dso_local local_unnamed_addr global i64 0, align 8

define dso_local i64 @_Z3foov() local_unnamed_addr #0 {
entry:
  %0 = load i64, ptr @b, align 8
  ret i64 %0
}

define dso_local i32 @main() local_unnamed_addr #1 {
omp.inner.for.body.lr.ph:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [256 x i64], align 16
  store i64 3, ptr @b, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %.omp.iv.local.09 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %omp.inner.for.body ]
  %call = call i64 @_Z3foov() #3
  %arrayidx = getelementptr inbounds [256 x i64], ptr %a, i64 0, i64 %.omp.iv.local.09
  store i64 %call, ptr %arrayidx, align 8
  %add1 = add nuw nsw i64 %.omp.iv.local.09, 1
  %exitcond = icmp eq i64 %add1, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2__Z3foov,_ZGVcN2__Z3foov,_ZGVdN2__Z3foov,_ZGVeN2__Z3foov,_ZGVbM2__Z3foov,_ZGVcM2__Z3foov,_ZGVdM2__Z3foov,_ZGVeM2__Z3foov" }
attributes #1 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
