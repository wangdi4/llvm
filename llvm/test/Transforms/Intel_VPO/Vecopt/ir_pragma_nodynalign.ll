; Test to check that #pragma vector nodynamic_align effectively disables dynamic peeling
; i.e., in presence of the pragma, the results of decision on making dynamic
; peeling does not depend on the corresponding switch, -vplan-enable-peeling.

; RUN: opt %s -disable-output -passes="vplan-vec" -vplan-enable-peeling=1 -mattr=+avx2 -debug-only=LoopVectorizationPlanner 2>&1 | FileCheck %s

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Selected peeling: Disabled
; CHECK: Peeling will not be performed

define dso_local void @_Z4initPll(ptr nocapture %in, i64 %N) local_unnamed_addr #0 {
entry:
  %cmp3.not14 = icmp slt i64 %N, 1
  br i1 %cmp3.not14, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.015 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %in, i64 %.omp.iv.local.015
  store i64 %.omp.iv.local.015, ptr %arrayidx, align 8
  %add5 = add nuw nsw i64 %.omp.iv.local.015, 1
  %exitcond.not = icmp eq i64 %add5, %N
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body, !llvm.loop !1

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

!1 = distinct !{!1,!2}
!2 = !{!"llvm.loop.intel.vector.nodynamic_align", !"true"}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

