; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check that we insert the implicit kmpc_barrier for "for" loops outside the zero-trip-test.
;
; Test src:
;
; #include <stdio.h>
; #ifndef N
; #define N 0
; #endif
; int main() {
; int x = 0;
; #pragma omp parallel
;  {
;  x = 0;
;  volatile int ub = N;
; #pragma omp for
;  for (int i = 0; i < ub; i++) {}
; #pragma omp master
;  x = 100;
;  }
;  printf("%d\n", x);
; } 
;
; CHECK: %cmp = icmp slt i32 0, %{{.*}}
; CHECK: br i1 %cmp, label %[[PRECONDTHEN:[^,]+]], label %[[PRECONDEND:[^,]+]]
; CHECK: [[PRECONDTHEN]]:
; CHECK: call void @__kmpc_for_static_init_4(
; CHECK: call void @__kmpc_for_static_fini(
; CHECK: br label %[[PRECONDEND:[^,]+]]
; CHECK: [[PRECONDEND]]:
; CHECK: call void @__kmpc_barrier(
; CHECK: br label %[[MASTER:[^,]+]]
; CHECK: [[MASTER]]: 
; CHECK: %{{.*}} = call i32 @__kmpc_masked(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %x, align 4
  store volatile i32 0, ptr %ub, align 4
  %1 = load i32, ptr %ub, align 4
  store i32 %1, ptr %.capture_expr.0, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %3 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %4 = load i32, ptr %.capture_expr.1, align 4
  store i32 %4, ptr %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %6 = load i32, ptr %.omp.lb, align 4
  store i32 %6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, ptr %.omp.iv, align 4
  %8 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %10, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]

  fence acquire
  store i32 100, ptr %x, align 4
  fence release
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %12 = load i32, ptr %x, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %12)
  %13 = load i32, ptr %retval, align 4
  ret i32 %13
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

