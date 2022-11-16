; RUN: opt -passes="vplan-vec" -debug-only=LoopVectorizationPlanner --disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-IR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -debug-only=LoopVectorizationPlanner --disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR
; REQUIRES: asserts

;; Check that we do not select VF=1 for the simd loop in both HIR/IR.
;; The min data type width for this loop is 64, the max is 80.
;; TTI returns MinVectorWidth=128, so MinVF = max(128/80, 1) = 1.

;; Difference in MinVF and MaxVF for HIR path vs. IR path is due to the fact,
;; that analysis responsible for calculating min and max data type width used
;; in a loop is not implemented for HIR path and replaced with constant common
;; values.
;; TODO: remove this comment once it is implemented in HIR.

; CHECK-IR: LVP: Orig MinVF: 1 Orig MaxVF: 4
; CHECK-IR: LVP: MinVF: 1 MaxVF: 4
; CHECK-IR: Selecting VPlan with VF=2{{[[:space:]]}}

; CHECK-HIR: LVP: Orig MinVF: 2 Orig MaxVF: 32
; CHECK-HIR: LVP: MinVF: 2 MaxVF: 32
; CHECK-HIR: Selecting VPlan with VF=2{{[[:space:]]}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPeS_S_(ptr %A, ptr %B, ptr %C) #0 {
DIR.OMP.SIMD.1:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.117

DIR.OMP.SIMD.117:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.117, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.117 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds x86_fp80, ptr %A, i64 %indvars.iv
  %1 = load x86_fp80, ptr %arrayidx, align 16
  %arrayidx2 = getelementptr inbounds x86_fp80, ptr %B, i64 %indvars.iv
  %2 = load x86_fp80, ptr %arrayidx2, align 16
  %add3 = fadd fast x86_fp80 %2, %1
  %arrayidx5 = getelementptr inbounds x86_fp80, ptr %C, i64 %indvars.iv
  store x86_fp80 %add3, ptr %arrayidx5, align 16
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

attributes #0 = { "target-cpu"="skylake-avx512" }
