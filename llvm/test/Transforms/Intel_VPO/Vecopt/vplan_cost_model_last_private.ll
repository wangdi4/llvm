; RUN: opt < %s -vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -disable-output -vplan-enable-masked-vectorized-remainder -vplan-enable-evaluators-cost-model-dumps -vplan-cost-model-print-analysis-for-vf=4 -vplan-force-vf=4 | FileCheck %s

; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -mtriple=x86_64-unknown-unknown -mattr=+avx2 -disable-output -vplan-enable-masked-vectorized-remainder -vplan-enable-evaluators-cost-model-dumps -vplan-cost-model-print-analysis-for-vf=4 -vplan-force-vf=4 | FileCheck %s

; The test captures the costs of private-final-c-mem, private-final-uc-mem and private-final-masked-mem VPInstructions.

; Main loop
; CHECK: Cost 13 for i8 %vp{{[0-9]+}} = private-final-c-mem i8 %vp{{[0-9]+}} i32 %vp{{[0-9]+}} i8* %ret.c.lpriv
; CHECK: Cost 1 for i8 %vp{{[0-9]+}} = private-final-uc-mem i8 %vp{{[0-9]+}}
; Reminder loop
; CHECK: Cost 13 for i8 %vp{{[0-9]+}} = private-final-c-mem i8 %vp{{[0-9]+}} i32 %vp{{[0-9]+}} i8* %ret.c.lpriv
; CHECK: Cost 7 for i8 %vp{{[0-9]+}} = private-final-masked-mem i8 %vp{{[0-9]+}} i1 %vp{{[0-9]+}}

define void @foo(i8* %a, i32 %ub) {
entry:
  %ret.c.lpriv = alloca i8
  %ret.uc.lpriv = alloca i8
  br label %0

0:
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:CONDITIONAL.TYPED"(i8* %ret.c.lpriv, i8 0, i32 1), "QUAL.OMP.LASTPRIVATE:TYPED"(i8* %ret.uc.lpriv, i8 0, i32 1)]
  br label %omp.inner.for.body

omp.inner.for.body:
  %.omp.iv.local.016 = phi i32 [ 0, %0 ], [ %add, %omp.inner.for.inc ]
  %cmp = icmp slt i32 0, %.omp.iv.local.016
  br i1 %cmp, label %if.then, label %omp.inner.for.inc

if.then:
  %2 = load i8, i8* %a
  store i8 %2, i8* %ret.c.lpriv
  br label %omp.inner.for.inc

omp.inner.for.inc:
  %3 = load i8, i8* %a
  store i8 %3, i8* %ret.uc.lpriv

  %add = add nsw i32 %.omp.iv.local.016, 1
  %cmp2 = icmp sgt i32 %ub, %add
  br i1 %cmp2, label %omp.inner.for.body, label %endloop

endloop:
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
