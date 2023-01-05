; RUN: opt -passes="vplan-vec" -debug-only=LoopVectorizationPlanner --disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -debug-only=LoopVectorizationPlanner --disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;; Check that we bail out early when simdlen of 1 is used.

; CHECK: LVP: ForcedVF: 1
; CHECK: LVP: The forced VF or safelen specified by user is 1, VPlans need not be constructed.

define void @_Z3fooPfS_S_(ptr %A, ptr %B, ptr %C) {
DIR.OMP.SIMD.1:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.117

DIR.OMP.SIMD.117:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 1), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.117, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.117 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  %2 = load float, ptr %arrayidx2, align 4
  %add3 = fadd fast float %2, %1
  %arrayidx5 = getelementptr inbounds float, ptr %C, i64 %indvars.iv
  store float %add3, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.116, label %omp.inner.for.body

DIR.OMP.END.SIMD.116:                             ; preds = %omp.inner.for.body
  store i32 1023, ptr %i.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.116
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
