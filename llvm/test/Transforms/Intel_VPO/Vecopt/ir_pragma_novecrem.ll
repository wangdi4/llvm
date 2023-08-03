; Test to check that #pragma vector novecremainder effectively disables remainder
; vectorization i.e., in presence of the pragma, the results of decision on remainder
; vectorization does not depend on the corresponding switches
; (-vplan-enable-non-masked-vectorized-remainder and -vplan-enable-masked-vectorized-remainder).

; RUN: opt %s -disable-output -passes="vplan-vec" -vplan-enable-masked-variant -march=core-avx2 -debug-only=LoopVectorizationPlanner
; RUN: opt %s -disable-output -passes="vplan-vec" -vplan-enable-non-masked-vectorized-remainder -vplan-enable-masked-vectorized-remainder -vplan-enable-masked-variant -march=core-avx2 -debug-only=LoopVectorizationPlanner
; REQUIRES: asserts

; CHECK: No vector remainder enabled

; Function Attrs: mustprogress nounwind uwtable
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
!2 = !{!"llvm.loop.intel.vector.novecremainder", !"true"}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

