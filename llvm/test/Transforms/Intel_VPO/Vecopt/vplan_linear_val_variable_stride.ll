; RUN: opt -vplan-vec -S < %s  | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vplan-vec -hir-cg -S < %s  | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check for val variable stride variant matching for both LLVM-IR and HIR

; CHECK: call noundef <8 x i64> @_ZGVbN8uLs0__Z3fooiRl

define i32 @main(i32 noundef %argc, i8** nocapture noundef readonly %argv) {
DIR.OMP.SIMD.2:
  %i.linear.iv = alloca i64, align 8
  %b = alloca [128 x i64], align 16
  %arrayidx = getelementptr inbounds i8*, i8** %argv, i64 1
  %0 = load i8*, i8** %arrayidx, align 8
  %call = tail call i32 @atoi(i8* nocapture noundef %0) #6
  %conv = sext i32 %call to i64
  %sub1 = add nsw i64 %conv, 127
  %div = sdiv i64 %sub1, %conv
  %cmp.not30 = icmp slt i64 %div, 1
  br i1 %cmp.not30, label %DIR.OMP.END.SIMD.429, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV"(i64* %i.linear.iv, i32 %call) ]
  br label %DIR.OMP.SIMD.137

DIR.OMP.SIMD.137:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.137, %omp.inner.for.body
  %.omp.iv.local.031 = phi i64 [ 0, %DIR.OMP.SIMD.137 ], [ %add8, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.031, %conv
  store i64 %mul, i64* %i.linear.iv, align 8
  %call6 = call noundef i64 @_Z3fooiRl(i32 noundef %call, i64* noundef nonnull align 8 dereferenceable(8) %i.linear.iv)
  %2 = load i64, i64* %i.linear.iv, align 8
  %arrayidx7 = getelementptr inbounds [128 x i64], [128 x i64]* %b, i64 0, i64 %2
  store i64 %call6, i64* %arrayidx7, align 8
  %add8 = add nuw nsw i64 %.omp.iv.local.031, 1
  %exitcond.not = icmp eq i64 %add8, %div
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
declare i64 @_Z3fooiRl(i32 noundef, i64* noundef nonnull align 8 dereferenceable(8)) local_unnamed_addr #0
declare i32 @atoi(i8* noundef)

attributes #0 = { "vector-variants"="_ZGVbN8uLs0__Z3fooiRl" }
