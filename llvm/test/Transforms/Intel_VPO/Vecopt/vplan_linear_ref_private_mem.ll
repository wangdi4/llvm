; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test matching of vector variant call using reference arg with ref modifier
; that is private memory. Stride for both caller/callee is 1.

; CHECK: call noundef <8 x i32> @_ZGVbN8R4__Z3fooRi

; for (i=0; i<128; i++)
;   a[i] = foo(i); // i is passed as reference and memory is private
; Since i is private and widened to VF, address of i is linear stride 1

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
DIR.OMP.SIMD.123:
  %i.linear.iv = alloca i32, align 4
  %a = alloca [128 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.123
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.016 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add1, %omp.inner.for.body ]
  store i32 %.omp.iv.local.016, ptr %i.linear.iv, align 4
  %call = call noundef i32 @_Z3fooRi(ptr noundef nonnull align 4 dereferenceable(4) %i.linear.iv)
  %1 = load i32, ptr %i.linear.iv, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [128 x i32], ptr %a, i64 0, i64 %idxprom
  store i32 %call, ptr %arrayidx, align 4
  %add1 = add nuw nsw i32 %.omp.iv.local.016, 1
  %exitcond.not = icmp eq i32 %add1, 128
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.414, label %omp.inner.for.body

DIR.OMP.END.SIMD.414:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.414
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i32 @_Z3fooRi(ptr noundef nonnull align 4 dereferenceable(4)) #3

attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8R4__Z3fooRi" }
