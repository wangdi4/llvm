;
; Test to check that cost model chooses masked mode loop.
;
; RUN: opt -passes='vplan-vec,print' -vplan-masked-main-cost-threshold=0 -disable-output -vplan-enable-masked-main-loop=1 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-directive-cleanup,hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-masked-main-cost-threshold=0 -disable-output -vplan-enable-masked-main-loop=1 < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Loop TC is 7 so checking for vector with size 8 is enough.
;CHECK: <8 x i32>

; Function Attrs: mustprogress nounwind uwtable
define dso_local i32 @_Z3fooPiii(ptr nocapture readnone %ptr, i32 %step, i32 %n) local_unnamed_addr #0 {
entry:
  call void @llvm.intel.directive.elementsize(ptr nonnull %ptr, i64 4)
  %s.red = alloca i32, align 4
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store i32 0, ptr %s.red, align 4
  br label %DIR.OMP.SIMD.128

DIR.OMP.SIMD.128:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %s.red, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.128
  %s.red.promoted = load i32, ptr %s.red, align 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %r1 = phi i32 [ %s.red.promoted, %DIR.OMP.SIMD.2 ], [ %add7, %omp.inner.for.body ]
  %.omp.iv.local.019 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add8, %omp.inner.for.body ]
  %p = getelementptr inbounds i32, ptr %ptr, i32 %.omp.iv.local.019
  %l2 = load i32, ptr %p, align 4
  %mul6 = mul nsw i32 %l2, %step
  %add7 = add nsw i32 %r1,  %mul6
  %add8 = add nuw nsw i32 %.omp.iv.local.019, 1
  %exitcond.not = icmp eq i32 %add8, 7
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %add7.lcssa = phi i32 [ %add7, %omp.inner.for.body ]
  store i32 %add7.lcssa, ptr %s.red, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  %s.1 = phi i32 [ 0, %entry ], [ %add7.lcssa, %DIR.OMP.END.SIMD.3 ]
  ret i32 %s.1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare void @llvm.intel.directive.elementsize(ptr, i64 immarg)

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

