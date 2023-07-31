; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/callee parameter matching selects the variant with the best score.
; See comment below at the call site for more detail.

; CHECK-LABEL: vector.body:
; CHECK: call <2 x i64> @_ZGVbN2vlv__Z3foolll

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i64 @_Z3foolll(i64 %n, i64 %i, i64 %x) local_unnamed_addr #0 {
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

; DA identifies arguments at this call-site as uniform (non-pointer), linear, and random.
; Variant parameter encodings available are vvv, uvv, and vlv.
; vvv Score = 2 (uniform->vector) + 2 (linear->vector) + 4 (random->vector)              = 8
; uvv Score = 3 (uniform->uniform non-pointer) + 2 (linear->vector) + 4 (random->vector) = 9
; vlv Score = 2 (uniform->vector) + 4 (linear->linear) + 4 (random->vector)              = 10
; So, the vlv variant is selected

  %call = call i64 @_Z3foolll(i64 256, i64 %.omp.iv.local.010, i64 %1) #3
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

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2vvv__Z3foolll,_ZGVcN4vvv__Z3foolll,_ZGVdN4vvv__Z3foolll,_ZGVeN8vvv__Z3foolll,_ZGVbM2vvv__Z3foolll,_ZGVcM4vvv__Z3foolll,_ZGVdM4vvv__Z3foolll,_ZGVeM8vvv__Z3foolll,_ZGVbN2uvv__Z3foolll,_ZGVcN4uvv__Z3foolll,_ZGVdN4uvv__Z3foolll,_ZGVeN8uvv__Z3foolll,_ZGVbM2uvv__Z3foolll,_ZGVcM4uvv__Z3foolll,_ZGVdM4uvv__Z3foolll,_ZGVeM8uvv__Z3foolll,_ZGVbN2vlv__Z3foolll,_ZGVcN4vlv__Z3foolll,_ZGVdN4vlv__Z3foolll,_ZGVeN8vlv__Z3foolll,_ZGVbM2vlv__Z3foolll,_ZGVcM4vlv__Z3foolll,_ZGVdM4vlv__Z3foolll,_ZGVeM8vlv__Z3foolll" }
attributes #1 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
