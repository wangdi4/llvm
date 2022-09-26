; RUN: opt -vplan-vec -S < %s  | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vplan-vec -hir-cg -S < %s  | FileCheck %s

; Test to check that the vector variant is called for a parameter that is variable
; linear integer stride. i.e., the s encoding for vector variants. In this test
; DA recognizes the hard-coded stride (2) for the linear argument on the caller
; side and variant matching is able to match that with the integer variable stride
; encoding on the callee side.

; CHECK: call noundef <8 x i64> @_ZGVbN8uls0__Z3foosl

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind readnone willreturn uwtable
define dso_local noundef i64 @_Z3foosl(i16 noundef signext %c, i64 noundef %x) local_unnamed_addr #0 {
entry:
  %add = add nsw i64 %x, 1
  ret i64 %add
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %argc, i8** nocapture noundef readnone %argv) local_unnamed_addr #1 {
DIR.OMP.SIMD.1:
  %a = alloca [128 x i64], align 16
  br label %DIR.OMP.SIMD.154

DIR.OMP.SIMD.154:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.154
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %1 = mul nuw nsw i64 %indvars.iv, 2
  %call = call noundef i64 @_Z3foosl(i16 noundef signext 2, i64 noundef %1)
  %arrayidx11 = getelementptr inbounds [128 x i64], [128 x i64]* %a, i64 0, i64 %1
  store i64 %call, i64* %arrayidx11, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.442, label %omp.inner.for.body

DIR.OMP.END.SIMD.442:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cond.cleanup16

for.cond.cleanup16:                               ; preds = %for.cond.cleanup16
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variants"="_ZGVbN8uls0__Z3foosl,_ZGVcN8uls0__Z3foosl,_ZGVdN8uls0__Z3foosl,_ZGVeN8uls0__Z3foosl,_ZGVbM8uls0__Z3foosl,_ZGVcM8uls0__Z3foosl,_ZGVdM8uls0__Z3foosl,_ZGVeM8uls0__Z3foosl,_ZGVbN8uu__Z3foosl,_ZGVcN8uu__Z3foosl,_ZGVdN8uu__Z3foosl,_ZGVeN8uu__Z3foosl,_ZGVbM8uu__Z3foosl,_ZGVcM8uu__Z3foosl,_ZGVdM8uu__Z3foosl,_ZGVeM8uu__Z3foosl" }
