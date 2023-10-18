; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter' -intel-opt-report=high -disable-output -intel-opt-report-file=stdout < %s | FileCheck %s

; Generating this remark used to cause an assert fail; this is a regression test
; to ensure this remark can be emitted correctly.

; CHECK: remark #15521: HIR: simd loop was not vectorized: loop control variable was not identified. Explicitly compute the iteration count before executing the loop or try using canonical loop form from OpenMP specification 5.0

define void @_ZN5NBodyIfE12execute_partEiii.DIR.OMP.PARALLEL.LOOP.2.split159() {
DIR.OMP.SIMD.7:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %cmp61.not = icmp ugt i32 0, 0
  br i1 %cmp61.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.6.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.6.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
