; RUN: opt %s -S -passes="hir-vplan-vec" -debug-only=VPlanCallVecDecisions -disable-output 2>&1 | FileCheck %s

; VPlanCallVecDecisions test to make sure it generates all combinations of caller side
; variants and that it matches the correct one.

; CHECK: _ZGV_unknown_N8ul4l4__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8ul4R4__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8ul4U__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8ul4L__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8uR4l4__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8uR4R4__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8uR4U__Z3fooiPiRi
; CHECK: _ZGV_unknown_N8uR4L__Z3fooiPiRi
; CHECK: Matched call to: _ZGVbN8ul4R4__Z3fooiPiRi

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
DIR.OMP.SIMD.127:
  %i.linear.iv = alloca i32, align 4
  %a = alloca [128 x i32], align 16
  %b = alloca [128 x i32], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.127
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds [128 x i32], ptr %b, i64 0, i64 %indvars.iv
  %call = call noundef i32 @_Z3fooiPiRi(i32 noundef 1, ptr noundef nonnull %arrayidx, ptr noundef nonnull align 4 dereferenceable(4) %i.linear.iv)
  %2 = load i32, ptr %i.linear.iv, align 4
  %idxprom1 = sext i32 %2 to i64
  %arrayidx2 = getelementptr inbounds [128 x i32], ptr %a, i64 0, i64 %idxprom1
  store i32 %call, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 128
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.416, label %omp.inner.for.body

DIR.OMP.END.SIMD.416:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.416
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local noundef i32 @_Z3fooiPiRi(i32 noundef, ptr noundef, ptr noundef nonnull align 4 dereferenceable(4)) local_unnamed_addr #3

attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8ul4R4__Z3fooiPiRi,_ZGVcN8ul4R4__Z3fooiPiRi,_ZGVdN8ul4R4__Z3fooiPiRi,_ZGVeN8ul4R4__Z3fooiPiRi,_ZGVbM8ul4R4__Z3fooiPiRi,_ZGVcM8ul4R4__Z3fooiPiRi,_ZGVdM8ul4R4__Z3fooiPiRi,_ZGVeM8ul4R4__Z3fooiPiRi" }
