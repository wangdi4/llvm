; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check to see that variable stride linear reference variant with ref modifier
; is called from both LLVM-IR and HIR.
; CHECK: call noundef <8 x i64> @_ZGVbN8uRs0__Z3fooiRl

;  for (long i=0; i<N; i+=c)
;    b[i] = foo(c, a[i]); // stride is c

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #0 {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  %b = alloca [128 x i64], align 16
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx, align 8
  %call = tail call i32 @atoi(ptr nocapture noundef %0) #6
  %conv = sext i32 %call to i64
  %sub1 = add nsw i64 %conv, 127
  %div = sdiv i64 %sub1, %conv
  %cmp.not30 = icmp slt i64 %div, 1
  br i1 %cmp.not30, label %DIR.OMP.END.SIMD.429, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %i.linear.iv, i64 0, i64 1, i32 %call) ]
  br label %DIR.OMP.SIMD.137

DIR.OMP.SIMD.137:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.137, %omp.inner.for.body
  %.omp.iv.local.031 = phi i64 [ 0, %DIR.OMP.SIMD.137 ], [ %add9, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.031, %conv
  %arrayidx6 = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %mul
  %call7 = call noundef i64 @_Z3fooiRl(i32 noundef %call, ptr noundef nonnull align 8 dereferenceable(8) %arrayidx6) #3
  %arrayidx8 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %mul
  store i64 %call7, ptr %arrayidx8, align 8
  %add9 = add nuw nsw i64 %.omp.iv.local.031, 1
  %exitcond.not = icmp eq i64 %add9, %div
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.429

DIR.OMP.END.SIMD.429:                             ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.429
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @atoi(ptr nocapture noundef) local_unnamed_addr
declare dso_local noundef i64 @_Z3fooiRl(i32 noundef, ptr noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #4

attributes #4 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8uRs0__Z3fooiRl" }
