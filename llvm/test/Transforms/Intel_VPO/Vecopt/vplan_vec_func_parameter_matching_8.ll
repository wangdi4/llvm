; RUN: opt -S -passes="vplan-vec" < %s | FileCheck %s

; Test that caller/callee simd function matching is successful for two variants and
; tiebreaker used is the MaxArg index, which represents the index of the argument
; that has the highest score. See more detailed comments at the call site below.

; CHECK-LABEL: vector.body:
; CHECK: call <2 x i64> @_ZGVbN2vvl__Z3foolll

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i64 @_Z3foolll(i64 %n, i64 %i, i64 %x) local_unnamed_addr #0 {
entry:
  %add = add nsw i64 %i, %n
  %add1 = add nsw i64 %add, %x
  ret i64 %add1
}

define dso_local i32 @main() local_unnamed_addr #1 {
omp.inner.for.body.lr.ph:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [256 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %.omp.iv.local.09 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %omp.inner.for.body ]

; At this call-site DA recognizes arguments as uniform, linear, and linear. There are two simd
; variant parameter signatures, vvl and vlv. They are scored as follows:
; vvl Score = 2 (uniform->vector) + 2 (linear->vector) + 4 (linear->linear) = 8, MaxArg = 2
; vlv Score = 2 (uniform->vector) + 4 (linear->linear) + 2 (linear->vector) = 8, MaxArg = 1
; Since MaxArg for vvl > MaxArg for vlv, then the vvl variant is selected.

  %call = call i64 @_Z3foolll(i64 256, i64 %.omp.iv.local.09, i64 %.omp.iv.local.09) #3
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

attributes #0 = { noinline norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN2vvl__Z3foolll,_ZGVcN2vvl__Z3foolll,_ZGVdN2vvl__Z3foolll,_ZGVeN2vvl__Z3foolll,_ZGVbM2vvl__Z3foolll,_ZGVcM2vvl__Z3foolll,_ZGVdM2vvl__Z3foolll,_ZGVeM2vvl__Z3foolll,_ZGVbN2vlv__Z3foolll,_ZGVcN2vlv__Z3foolll,_ZGVdN2vlv__Z3foolll,_ZGVeN2vlv__Z3foolll,_ZGVbM2vlv__Z3foolll,_ZGVcM2vlv__Z3foolll,_ZGVdM2vlv__Z3foolll,_ZGVeM2vlv__Z3foolll" }
attributes #1 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
