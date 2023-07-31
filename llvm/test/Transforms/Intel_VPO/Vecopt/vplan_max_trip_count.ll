; REQUIRES: asserts
; RUN: opt -S -passes=vplan-vec -debug-only=VPlanHCFGBuilder -disable-output < %s 2>&1 | FileCheck %s

;; Estimate max TC for the loop using ScalarEvolution.

;; void foo(unsigned *a, unsigned *b, bool Pred) {
;;     unsigned TC;
;;     if (Pred)
;;       TC = 200;
;;     else
;;       TC = 300;
;; #pragma omp simd simdlen(4)
;;     for(unsigned i = 0; i < TC; i++) {
;;         a[i] = b[i] * b[i];
;;     }
;; }

; CHECK: The max trip count for loop omp.inner.for.body is estimated to be 300
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z3fooPjS_b(ptr nocapture %a, ptr nocapture readonly %b, i1 zeroext %Pred) {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %1 = select i1 %Pred, i64 200, i64 300
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx, align 4
  %mul7 = mul i32 %3, %3
  %arrayidx9 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 %mul7, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %omp.precond.end.loopexit, label %omp.inner.for.body

omp.precond.end.loopexit:                         ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.precond.end.loopexit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
