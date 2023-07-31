; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/caller parameter matching results in call to simd variant.

; CHECK-LABEL: vector.body:
; CHECK: call <2 x i64> @_ZGVbN2ulv_foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i64 @foo(i64 %n, i64 %i, i64 %x) local_unnamed_addr #0 {
entry:
  %add = add i64 %i, %n
  %add1 = add i64 %add, %x
  ret i64 %add1
}

define dso_local i32 @main() local_unnamed_addr #1 {
omp.inner.for.body.lr.ph:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [256 x i64], align 16
  %b = alloca [256 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %.omp.iv.local.010 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add2, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds [256 x i64], ptr %b, i64 0, i64 %.omp.iv.local.010
  %1 = load i64, ptr %arrayidx, align 8
  %call = call i64 @foo(i64 256, i64 %.omp.iv.local.010, i64 %1) #3
  %arrayidx1 = getelementptr inbounds [256 x i64], ptr %a, i64 0, i64 %.omp.iv.local.010
  store i64 %call, ptr %arrayidx1, align 8
  %add2 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond = icmp eq i64 %add2, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 0
}

attributes #0 = { "vector-variants"="_ZGVbN2ulv_foo,_ZGVcN4ulv_foo,_ZGVdN4ulv_foo,_ZGVeN8ulv_foo,_ZGVbM2ulv_foo,_ZGVcM4ulv_foo,_ZGVdM4ulv_foo,_ZGVeM8ulv_foo" noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
