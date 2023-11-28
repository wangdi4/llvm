; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int i = 10;
;
; int foo() {
  ; #pragma omp simd linear(i:2)
;   for (i = 0; i < 10; i+=2) {
;   }
;   return i;
; }
;
; /*int main() {
;   int result = foo();
;   if (result != 10) {
;     printf("failed. i = %d (expected 10)\n", result);
;     return 1;
;   }
;   printf("passed\n");
;   return 0;
; }*/

; Check that WRegion construction correctly parses LINEAR:TYPED
; CHECK: LINEAR clause (size=1): (TYPED(ptr @i, TYPE: i32, NUM_ELEMENTS: i32 1), i32 2)
; Check that Paropt doesn't transform LINEAR:TYPED var on a SIMD directiv
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED"(ptr @i, i32 0, i32 1, i32 2){{.*}}]

; ModuleID = 'simd_linear_iv.c'
source_filename = "simd_linear_iv.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = dso_local global i32 10, align 4

define dso_local i32 @foo() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 4, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr @i, i32 0, i32 1, i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 2
  %add = add nsw i32 0, %mul
  store i32 %add, ptr @i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %4 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %4, 1
  store i32 %add1, ptr %.omp.iv, align 4
  %5 = load i32, ptr @i, align 4
  %add2 = add nsw i32 %5, 2
  store i32 %add2, ptr @i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  %6 = load i32, ptr @i, align 4
  ret i32 %6
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
