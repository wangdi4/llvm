; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo(void*);
;
; void negative_test1(int N, int M) {
;   int D;
;   for (int J = 0; J < M; ++J) {
; #pragma omp parallel firstprivate(D)
;     { foo(&D); }
; #pragma omp parallel for lastprivate(D)
;     for (int I = 0; I < N; ++I) { foo(&D); }
;   }
; }

; CHECK-NOT: remark:{{.*}} LASTPRIVATE clause for variable 'D' on 'parallel loop' construct is redundant

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare dso_local void @foo(ptr noundef)

define dso_local void @negative_test1(i32 noundef %N, i32 noundef %M) {
; CHECK-LABEL: @negative_test1(
entry:
  %N.addr = alloca i32, align 4
  %M.addr = alloca i32, align 4
  %D = alloca i32, align 4
  %J = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.7 = alloca i32, align 4
  %.capture_expr.8 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  store i32 %M, ptr %M.addr, align 4
  store i32 0, ptr %J, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %J, align 4
  %1 = load i32, ptr %M.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1) ]
  call void @foo(ptr noundef %D)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  %3 = load i32, ptr %N.addr, align 4
  store i32 %3, ptr %.capture_expr.7, align 4
  %4 = load i32, ptr %.capture_expr.7, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.8, align 4
  %5 = load i32, ptr %.capture_expr.7, align 4
  %cmp3 = icmp slt i32 0, %5
  br i1 %cmp3, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %for.body
  store i32 0, ptr %.omp.lb, align 4
  %6 = load i32, ptr %.capture_expr.8, align 4
  store i32 %6, ptr %.omp.ub, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %8 = load i32, ptr %.omp.lb, align 4
  store i32 %8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %9 = load i32, ptr %.omp.iv, align 4
  %10 = load i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sle i32 %9, %10
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, ptr %I, align 4
  call void @foo(ptr noundef %D)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add6 = add nsw i32 %12, 1
  store i32 %add6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %for.body
  br label %for.inc

for.inc:                                          ; preds = %omp.precond.end
  %13 = load i32, ptr %J, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %J, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
