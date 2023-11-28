; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check for uval variable stride variant matching for both LLVM-IR and HIR

; CHECK: call noundef <8 x i64> @_ZGVbN8uUs0__Z3fooiRl

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #0 {
entry:
  %i9.linear.iv = alloca i64, align 8
  %b = alloca [128 x i64], align 16
  %arrayidx3 = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx3, align 8
  %call = tail call i32 @atoi(ptr nocapture noundef %0) #7
  %conv4 = sext i32 %call to i64
  %sub5 = add nsw i64 %conv4, 127
  %div = sdiv i64 %sub5, %conv4
  %cmp8.not47 = icmp slt i64 %div, 1
  br i1 %cmp8.not47, label %DIR.OMP.END.SIMD.446, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %i9.linear.iv, i64 0, i64 1, i32 %call) ]
  br label %DIR.OMP.SIMD.155

DIR.OMP.SIMD.155:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.155, %omp.inner.for.body
  %.omp.iv.local.048 = phi i64 [ 0, %DIR.OMP.SIMD.155 ], [ %add14, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.048, %conv4
  store i64 %mul, ptr %i9.linear.iv, align 8
  %call12 = call noundef i64 @_Z3fooiRl(i32 noundef %call, ptr noundef nonnull align 8 dereferenceable(8) %i9.linear.iv) #3
  %2 = load i64, ptr %i9.linear.iv, align 8
  %arrayidx13 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %2
  store i64 %call12, ptr %arrayidx13, align 8
  %add14 = add nuw nsw i64 %.omp.iv.local.048, 1
  %exitcond.not = icmp eq i64 %add14, %div
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.446

DIR.OMP.END.SIMD.446:                             ; preds = %entry, %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  br label %for.cond.cleanup18

for.cond.cleanup18:                               ; preds = %DIR.OMP.END.SIMD.446
  ret i32 0
}

declare dso_local i32 @atoi(ptr nocapture noundef) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i64 @_Z3fooiRl(i32 noundef, ptr noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #4

attributes #4 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8uUs0__Z3fooiRl" }
