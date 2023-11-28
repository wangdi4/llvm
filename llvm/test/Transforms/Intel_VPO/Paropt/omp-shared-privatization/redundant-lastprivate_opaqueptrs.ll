; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; void test1(int N) {
;   int A;
; #pragma omp simd lastprivate(A)
;   for (int I = 0; I < N; ++I) {}
; }

; This test checks that shared privatization pass emits diagnostic and replaces
; lastprivate clause with private if item has no uses inside or outside the work
; region.
;
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'A' on 'simd' construct is redundant

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test1(i32 noundef %N) {
; CHECK-LABEL: @test1(
entry:
  %N.addr = alloca i32, align 4
  %A = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %A, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %A, i32 0, i32 1)

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %8, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
