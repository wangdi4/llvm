; RUN: opt -passes=vplan-vec -S < %s  | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg -S < %s  | FileCheck %s

; Test to check that the vector variant is called for a parameter that is variable
; linear integer stride for pointer arg. i.e., the s encoding for vector variants.
; In this test DA recognizes the hard-coded stride (2) for the pointer on the caller
; side and variant matching is able to match that with the variable stride encoding
; on the callee side.

; CHECK: call noundef <8 x i64> @_ZGVbN8uls0__Z3foosPl

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i64 @_Z3foosPl(i16 noundef signext %c, ptr noundef %x) local_unnamed_addr #0 {
entry:
  %0 = load i64, ptr %x, align 8
  %add = add nsw i64 %0, 1
  ret i64 %add
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, ptr nocapture noundef readnone %argv) local_unnamed_addr #1 {
entry:
  %i8.linear.iv = alloca i64, align 8
  %a = alloca [128 x i64], align 16
  %b = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i8.linear.iv, i64 0, i32 1, i16 2) ]
  br label %DIR.OMP.SIMD.158

DIR.OMP.SIMD.158:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.158, %omp.inner.for.body
  %.omp.iv.local.047 = phi i64 [ 0, %DIR.OMP.SIMD.158 ], [ %add13, %omp.inner.for.body ]
  %mul = mul nuw nsw i64 %.omp.iv.local.047, 2
  %arrayidx11 = getelementptr inbounds [128 x i64], ptr %a, i64 0, i64 %mul
  %call = call noundef i64 @_Z3foosPl(i16 noundef signext 2, ptr noundef nonnull %arrayidx11) #3
  %arrayidx12 = getelementptr inbounds [128 x i64], ptr %b, i64 0, i64 %mul
  store i64 %call, ptr %arrayidx12, align 16
  %add13 = add nuw nsw i64 %.omp.iv.local.047, 1
  %exitcond54.not = icmp eq i64 %add13, 64
  br i1 %exitcond54.not, label %DIR.OMP.END.SIMD.445, label %omp.inner.for.body

DIR.OMP.END.SIMD.445:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup17

for.cond.cleanup17:                               ; preds = %DIR.OMP.END.SIMD.445
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbN8uls0__Z3foosPl,_ZGVcN8uls0__Z3foosPl,_ZGVdN8uls0__Z3foosPl,_ZGVeN8uls0__Z3foosPl,_ZGVbM8uls0__Z3foosPl,_ZGVcM8uls0__Z3foosPl,_ZGVdM8uls0__Z3foosPl,_ZGVeM8uls0__Z3foosPl" }
