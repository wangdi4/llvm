; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; VFInfo::get will assert if we try to generate a variant with uval/val modifier
; if the value is uniform. This test checks to make sure no assert happens.
; In this test, %N is uniform and is stored to the private memory location.

; Since we need to check for something, check that calls to the vector function
; are scalarized since the ptr stride doesn't match any of the caller variants
; with any of the vector-variants.
;
; CHECK:      call noundef i32 @_Z3barPii
; CHECK-NEXT: call noundef i32 @_Z3barPii
; CHECK-NEXT: call noundef i32 @_Z3barPii
; CHECK-NEXT: call noundef i32 @_Z3barPii

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z3fooi(i32 noundef %N) local_unnamed_addr #0 {
entry:
  %i.linear.iv = alloca i32, align 4
  %k.priv = alloca i32, align 4
  %cmp = icmp slt i32 %N, 1
  br i1 %cmp, label %omp.precond.end, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %i.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %k.priv, i32 0, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %.omp.iv.local.017 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  store i32 %.omp.iv.local.017, ptr %i.linear.iv, align 4
  store i32 %N, ptr %k.priv, align 4
  %call = call noundef i32 @_Z3barPii(ptr noundef nonnull %k.priv, i32 noundef %.omp.iv.local.017)
  %add5 = add nuw nsw i32 %.omp.iv.local.017, 1
  %exitcond.not = icmp eq i32 %add5, %N
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, %entry
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i32 @_Z3barPii(ptr noundef, i32 noundef) local_unnamed_addr #3

attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN4ul__Z3barPii" }
