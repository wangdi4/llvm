; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int n) {
;   int a = 1;
; #pragma omp taskloop num_tasks(n)
;   for (int i = 0; i < n; i++)
;   {
;     printf("%d\n", a);
;   }
; }

; Check that the task's outlined function doesn't contain any extra argument for the num_tasks operand '%4'.
; CHECK: define internal void @{{[^ ]+}}DIR.OMP.TASK{{[^ ]+}}(i32 {{[^ ]+}}, ptr {{[^ ]+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %a = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  store i32 1, ptr %a, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

  %1 = load i32, ptr %n.addr, align 4
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
  %5 = load i32, ptr %n.addr, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.NUM_TASKS"(i32 %5),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %n.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %7 = load i32, ptr %.omp.lb, align 4
  store i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %8 = load i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %8, %9
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %11 = load i32, ptr %a, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %11) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %12, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
