; test to check that tripcount value affects masked mode loop creation 

; RUN: opt %s -disable-output -passes="vplan-vec" -vplan-print-after-create-masked-vplan
; REQUIRES: asserts

; Function Attrs: mustprogress nounwind uwtable
; CHECK: VPlan after emitting masked variant
define dso_local void @_Z4initPll(ptr nocapture %in) local_unnamed_addr #0 {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.015 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %in, i64 %.omp.iv.local.015
  store i64 %.omp.iv.local.015, ptr %arrayidx, align 8
  %add5 = add nuw nsw i64 %.omp.iv.local.015, 1
  %exitcond.not = icmp eq i64 %add5, 7
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
; CHECK: VPlan after emitting masked variant
define dso_local void @_Z4init2Pll(ptr nocapture %in) local_unnamed_addr #0 {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.015 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %in, i64 %.omp.iv.local.015
  store i64 %.omp.iv.local.015, ptr %arrayidx, align 8
  %add5 = add nuw nsw i64 %.omp.iv.local.015, 1
  %exitcond.not = icmp eq i64 %add5, 1024
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

; Function Attrs: mustprogress nounwind uwtable
; CHECK-NOT: VPlan after emitting masked variant
define dso_local void @_Z4init3Pll(ptr nocapture %in) local_unnamed_addr #0 {
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),  "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.015 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %add5, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %in, i64 %.omp.iv.local.015
  store i64 %.omp.iv.local.015, ptr %arrayidx, align 8
  %add5 = add nuw nsw i64 %.omp.iv.local.015, 1
  %exitcond.not = icmp eq i64 %add5, 8
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

