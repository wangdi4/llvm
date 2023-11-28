; RUN: opt %s -S -passes=vplan-vec -vplan-print-after-plain-cfg -vplan-entities-dump 2>&1 | FileCheck %s

;; Test to check that last private with TYPED clause is imported to VPlan given opaque input.

define void @foo(ptr %a) {
; CHECK-LABEL: Private list
; CHECK-EMPTY:
; CHECK-NEXT:      Private tag: InMemory
; CHECK-NEXT:      Linked values: ptr %ret.lpriv,
; CHECK-NEXT:     Memory: ptr %ret.lpriv
entry:
  %ret.lpriv = alloca i8
  br label %0

0:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:CONDITIONAL.TYPED"(ptr %ret.lpriv, i8 0, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.016 = phi i32 [ 0, %0 ], [ %add9, %omp.inner.for.inc ]
  %cmp = icmp slt i32 0, %.omp.iv.local.016
  br i1 %cmp, label %if.then, label %omp.inner.for.inc

if.then:
  %2 = load i8, ptr %a
  store i8 %2, ptr %ret.lpriv
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
