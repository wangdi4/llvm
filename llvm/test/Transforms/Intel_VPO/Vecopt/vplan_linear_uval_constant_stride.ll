; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check for uval constant stride variant matching for both LLVM-IR and HIR

; CHECK: call noundef <8 x i64> @_ZGVbN8U2__Z3fooRl

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
DIR.OMP.SIMD.1:
  %i2.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.135

DIR.OMP.SIMD.135:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %i2.linear.iv, i64 0, i64 1, i32 2) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.135
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.028 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add4, %omp.inner.for.body ]
  %mul = shl nuw nsw i64 %.omp.iv.local.028, 1
  store i64 %mul, ptr %i2.linear.iv, align 8
  %call = call noundef i64 @_Z3fooRl(ptr noundef nonnull align 8 dereferenceable(8) %i2.linear.iv) #2
  %1 = load i64, ptr %i2.linear.iv, align 8
  %arrayidx3 = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %1
  store i64 %call, ptr %arrayidx3, align 8
  %add4 = add nuw nsw i64 %.omp.iv.local.028, 1
  %exitcond.not = icmp eq i64 %add4, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.426, label %omp.inner.for.body

DIR.OMP.END.SIMD.426:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup8

for.cond.cleanup8:                                ; preds = %DIR.OMP.END.SIMD.426
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i64 @_Z3fooRl(ptr noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #3

attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8U2__Z3fooRl" }
