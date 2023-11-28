;
; Test to check that we don't crash in GS-heuristc on invalid loop cost.
;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec' -print-after=hir-vplan-vec -enable-intel-advanced-opts %s 2>&1 | FileCheck %s

; CHECK:  %_ZGVbN4vv__Z5pointdd = @_ZGVbN4vv__Z5pointdd(3.300000e+00,  4.400000e+00);

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress uwtable
define dso_local void @_Z9MandelOMPv() local_unnamed_addr #0 {
DIR.OMP.SIMD.1:
  %j.linear.iv = alloca i32, align 4
  %ptr = alloca [1440 x double], align 8
  br label %DIR.OMP.SIMD.113

DIR.OMP.SIMD.113:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %j.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.113, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.113 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %call1 = call noundef i32 @_Z5pointdd(double noundef nofpclass(nan inf) 3.300000e+00, double noundef nofpclass(nan inf) 4.400000e+00)
  %1 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx = getelementptr inbounds i32, ptr %ptr, i64 %1
  store i32 %call1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 120
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local noundef i32 @_Z5pointdd(double noundef nofpclass(nan inf), double noundef nofpclass(nan inf)) local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = {  "vector-variants"="_ZGVbN4vv__Z5pointdd,_ZGVbM4vv__Z5pointdd,_ZGVcN8vv__Z5pointdd,_ZGVcM8vv__Z5pointdd,_ZGVdN8vv__Z5pointdd,_ZGVdM8vv__Z5pointdd,_ZGVeN16vv__Z5pointdd,_ZGVeM16vv__Z5pointdd" }
attributes #2 = { nounwind }
