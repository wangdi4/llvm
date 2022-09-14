; RUN: opt -vplan-vec -S < %s  | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vplan-vec -hir-cg -S < %s  | FileCheck %s

; Test to check that the vector variant is called for a parameter that is variable
; linear integer stride. i.e., the s encoding for vector variants. In this test
; DA recognizes the stride as unknown for the linear argument on the caller
; side and variant matching is able to match that with the integer variable stride
; encoding on the callee side.

; CHECK: call noundef <8 x i64> @_ZGVbN8uls0__Z3fooll

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind readnone willreturn uwtable
define dso_local noundef i64 @_Z3fooll(i64 noundef signext %c, i64 noundef %x) local_unnamed_addr #0 {
entry:
  %add = add nsw i64 %x, 1
  ret i64 %add
}

define dso_local noundef i32 @main(i32 noundef %argc, i8** nocapture noundef readonly %argv) local_unnamed_addr #1 {
entry:
  %a = alloca [128 x i64], align 16
  %arrayidx1 = getelementptr inbounds i8*, i8** %argv, i64 1
  %0 = load i8*, i8** %arrayidx1, align 8
  %call = tail call i64 @atol(i8* nocapture noundef %0) #8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %DIR.OMP.SIMD.148

DIR.OMP.SIMD.148:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.148, %omp.inner.for.body
  %.omp.iv.local.041 = phi i64 [ 0, %DIR.OMP.SIMD.148 ], [ %add9, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.041, %call
  %call7 = call noundef i64 @_Z3fooll(i64 noundef %call, i64 noundef %mul)
  %arrayidx8 = getelementptr inbounds [128 x i64], [128 x i64]* %a, i64 0, i64 %mul
  store i64 %call7, i64* %arrayidx8, align 4
  %add9 = add nuw nsw i64 %.omp.iv.local.041, 1
  %exitcond.not = icmp eq i64 %add9, 64
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.439

DIR.OMP.END.SIMD.439:                             ; preds = %entry, %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  br label %for.cond.cleanup13

for.cond.cleanup13:                               ; preds = %for.cond.cleanup13
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i64 @atol(i8* noundef)

attributes #0 = { "vector-variants"="_ZGVbN8uls0__Z3fooll,_ZGVcN8uls0__Z3fooll,_ZGVdN8uls0__Z3fooll,_ZGVeN8uls0__Z3fooll,_ZGVbM8uls0__Z3fooll,_ZGVcM8uls0__Z3fooll,_ZGVdM8uls0__Z3fooll,_ZGVeM8uls0__Z3fooll" }
