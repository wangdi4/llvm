; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; On the caller side, DA is able to recognize that stride is a constant
; even though stride is passed via another argument. This test checks to
; see that the constant stride variant is called for tie-breaking purposes.
; CHECK: call noundef <8 x i64> @_ZGVbN8uR16__Z3fooiRl

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readnone %argv) local_unnamed_addr #0 {
DIR.OMP.SIMD.137:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  %b = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.137
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i64 0, i32 1, i32 2) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.029 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  %mul = shl nuw nsw i64 %.omp.iv.local.029, 1
  %arrayidx = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %mul
  %call = call noundef i64 @_Z3fooiRl(i32 noundef 2, ptr noundef nonnull align 8 dereferenceable(8) %arrayidx) #2
  %arrayidx6 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %mul
  store i64 %call, ptr %arrayidx6, align 16
  %add7 = add nuw nsw i64 %.omp.iv.local.029, 1
  %exitcond.not = icmp eq i64 %add7, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.427, label %omp.inner.for.body

DIR.OMP.END.SIMD.427:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.427
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i64 @_Z3fooiRl(i32 noundef, ptr noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #3

attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8uR16__Z3fooiRl,_ZGVbN8uRs0__Z3fooiRl" }
