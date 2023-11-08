; Test to check that we don't crash on constant trip count loops on
; simd loops when large VF for calls is disabled.
;
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vplan-vec" -disable-output -print-after=hir-vplan-vec -vplan-cm-prohibit-zmm-low-pumping=1 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vplan-vec" -disable-output -print-after=hir-vplan-vec -vplan-cm-prohibit-zmm-low-pumping=0 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:           DO i1 = 0, 511, 8   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:        %_ZGVeN8vv_foo = @_ZGVeN8vv_foo(%A,  i1 + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>);
; CHECK-NEXT:      END LOOP

; Function Attrs: nounwind uwtable
define dso_local void @caller(ptr noundef %A) local_unnamed_addr #0 {
DIR.OMP.SIMD.112:
  %I.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.112
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8),  "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add2, %omp.inner.for.body ]
  %call1 = call fast nofpclass(nan inf) double @foo(ptr noundef %A, i32 %.omp.iv.local.0)
  %add2 = add nuw nsw i32 %.omp.iv.local.0, 1
  %exitcond.not = icmp eq i32 %add2, 512
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local nofpclass(nan inf) double @foo(ptr noundef, i32 noundef) local_unnamed_addr #2
attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "vector-variants"="_ZGVbN8vv_foo,_ZGVcN8vv_foo,_ZGVdN8vv_foo,_ZGVeN8vv_foo" }

