;
; RUN: opt %s -enable-new-pm=0 -disable-output -vplan-vec -debug-only=vpo-ir-loop-vectorize-legality 2>&1 | FileCheck %s
; RUN: opt %s -enable-new-pm=0 -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -debug-only=HIRLegality 2>&1 | FileCheck %s
; RUN: opt %s -disable-output -passes="vplan-vec" -debug-only=vpo-ir-loop-vectorize-legality 2>&1 | FileCheck %s
; RUN: opt %s -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -debug-only=HIRLegality 2>&1 | FileCheck %s

; CHECK: Conditional lastprivate of a vector type is not supported
; CHECK-NOT: <16 x i8>
;
define void @foo(<8 x i8>* %a) {
entry:
  %ret.lpriv = alloca <8 x i8>
  br label %0

0:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:CONDITIONAL"(<8 x i8>* %ret.lpriv), "QUAL.OMP.SIMDLEN"(i64 2) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.016 = phi i32 [ 0, %0 ], [ %add9, %omp.inner.for.inc ]
  %cmp = icmp slt i32 0, %.omp.iv.local.016
  br i1 %cmp, label %if.then, label %omp.inner.for.inc

if.then:
  %2 = load <8 x i8>, <8 x i8>* %a
  store <8 x i8> %2, <8 x i8>* %ret.lpriv
  br label %omp.inner.for.inc

omp.inner.for.inc:
  %add9 = add nsw i32 %.omp.iv.local.016, 1
  %cmp4 = icmp sgt i32 100, %add9
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.cond.omp.inner.for.end_crit_edge

omp.inner.for.cond.omp.inner.for.end_crit_edge:
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
