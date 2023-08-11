; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg -S < %s  | FileCheck %s

; Test to check that the vector variant is called for a parameter that is variable
; linear integer stride for pointer arg. i.e., the s encoding for vector variants.
; In this test DA recognizes the stride as unknown for the pointer on the caller
; side and variant matching is able to match that with the variable stride encoding
; on the callee side.

; CHECK: call noundef <8 x i64> @_ZGVbN8uls0__Z3fooiPl

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree noinline norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef i64 @_Z3fooiPl(i32 noundef %c, ptr nocapture noundef readonly %x) local_unnamed_addr #0 {
entry:
  %0 = load i64, ptr %x, align 8
  %add = add nsw i64 %0, 1
  ret i64 %add
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readonly %argv) local_unnamed_addr #1 {
entry:
  %i9.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  %b = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %arrayidx3 = getelementptr inbounds ptr, ptr %argv, i64 1
  %0 = load ptr, ptr %arrayidx3, align 8
  %call = tail call i32 @atoi(ptr nocapture noundef %0) #8
  %conv4 = sext i32 %call to i64
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i9.linear.iv, i64 0, i32 1, i32 %call) ]
  br label %DIR.OMP.SIMD.160

DIR.OMP.SIMD.160:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.160, %omp.inner.for.body
  %.omp.iv.local.049 = phi i64 [ 0, %DIR.OMP.SIMD.160 ], [ %add15, %omp.inner.for.body ]
  %mul = mul nsw i64 %.omp.iv.local.049, %conv4
  %arrayidx12 = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %mul
  %call13 = call noundef i64 @_Z3fooiPl(i32 noundef %call, ptr noundef nonnull %arrayidx12) #4
  %arrayidx14 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %mul
  store i64 %call13, ptr %arrayidx14, align 8
  %add15 = add nuw nsw i64 %.omp.iv.local.049, 1
  %exitcond56.not = icmp eq i64 %add15, 64
  br i1 %exitcond56.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.447

DIR.OMP.END.SIMD.447:                             ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  br label %for.cond.cleanup19

for.cond.cleanup19:                               ; preds = %DIR.OMP.END.SIMD.447
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @atoi(ptr nocapture noundef) local_unnamed_addr

attributes #0 = { "vector-variants"="_ZGVbN8uls0__Z3fooiPl,_ZGVcN8uls0__Z3fooiPl,_ZGVdN8uls0__Z3fooiPl,_ZGVeN8uls0__Z3fooiPl,_ZGVbM8uls0__Z3fooiPl,_ZGVcM8uls0__Z3fooiPl,_ZGVdM8uls0__Z3fooiPl,_ZGVeM8uls0__Z3fooiPl" }
