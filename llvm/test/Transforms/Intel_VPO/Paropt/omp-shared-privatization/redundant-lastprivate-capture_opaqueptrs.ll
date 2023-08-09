; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo(void*);
;
; void test3(int N) {
;   int D;
; #pragma omp parallel firstprivate(D)
;   { foo(&D); }
; #pragma omp parallel for lastprivate(D)
;   for (int I = 0; I < N; ++I) { foo(&D); }
; }

; This test checks that shared privatization pass emits diagnostic and replaces
; lastprivate clause with private if item has no uses inside or outside the work
; region.
;
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'D' on 'parallel loop' construct is redundant

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @test3(i32 noundef %N) {
; CHECK-LABEL: @test3(
entry:
  %N.addr = alloca i32, align 4
  %D = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1) ]
  call void @foo(ptr noundef %D)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.5, align 4
  %2 = load i32, ptr %.capture_expr.5, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.6, align 4
  %3 = load i32, ptr %.capture_expr.5, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %4 = load i32, ptr %.capture_expr.6, align 4
  store i32 %4, ptr %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1)

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
  store i32 %add4, ptr %I, align 4
  call void @foo(ptr noundef %D)
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
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare dso_local void @foo(ptr noundef)
