; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/callee parameter matching results in call to simd variant.

; CHECK-LABEL: vector.body:
; CHECK: call <4 x i32> @_ZGVbN4ulv_foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i32 @foo(i32 %n, i32 %i, i32 %x) local_unnamed_addr #0 {
entry:
  %add = add i32 %i, %n
  %add1 = add i32 %add, %x
  ret i32 %add1
}

define dso_local i32 @main() local_unnamed_addr #1 {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i32, align 4
  %a = alloca [256 x i32], align 16
  %b = alloca [256 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds [256 x i32], ptr %b, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %2 = trunc i64 %indvars.iv to i32
  %call = call i32 @foo(i32 256, i32 %2, i32 %1) #3
  %arrayidx2 = getelementptr inbounds [256 x i32], ptr %a, i64 0, i64 %indvars.iv
  store i32 %call, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 0
}

attributes #0 = { "vector-variants"="_ZGVbN4ulv_foo,_ZGVcN8ulv_foo,_ZGVdN8ulv_foo,_ZGVeN16ulv_foo,_ZGVbM4ulv_foo,_ZGVcM8ulv_foo,_ZGVdM8ulv_foo,_ZGVeM16ulv_foo" noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
