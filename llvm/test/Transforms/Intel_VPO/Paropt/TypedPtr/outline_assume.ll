; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; void foo(int *A, int N) {
; #pragma omp parallel
;   {
;     __builtin_assume(N > 0);
; #pragma omp for
;     for (int I = 0; I < N; ++I)
;       A[I] = I;
;   }
; }
;
; Check that __builtin_assume call is propagated into the outlined parallel region.
;
; CHECK: define void @foo(i32* %{{.+}}, i32 %{{.+}}) {
; CHECK:   call void{{.*}} @__kmpc_fork_call({{.+}}, i32 2, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i32*)* [[FOO_OUTLINED:@.+]] to void (i32*, i32*, ...)*),
; CHECK: }
;
; CHECK: define internal void [[FOO_OUTLINED]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i32* [[NADDR:%.+]]) #{{[0-9]+}} {
; CHECK:   [[VAL:%.+]] = load i32, i32* [[NADDR]]
; CHECK:   [[CMP:%.+]] = icmp sgt i32 [[VAL]], 0
; CHECK:   call void @llvm.assume(i1 [[CMP]])
; CHECK: }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %A, i32 %N) {
entry:
  %A.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %A, i32** %A.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32** %A.addr),
    "QUAL.OMP.SHARED"(i32* %N.addr),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %I),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %tmp) ]

  %1 = load i32, i32* %N.addr, align 4
  %cmp = icmp sgt i32 %1, 0
  call void @llvm.assume(i1 %cmp)
  %2 = load i32, i32* %N.addr, align 4
  store i32 %2, i32* %.capture_expr.0, align 4
  %3 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %4 = load i32, i32* %.capture_expr.0, align 4
  %cmp3 = icmp slt i32 0, %4
  br i1 %cmp3, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %5 = load i32, i32* %.capture_expr.1, align 4
  store i32 %5, i32* %.omp.ub, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %I) ]

  %7 = load i32, i32* %.omp.lb, align 4
  store i32 %7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %8 = load i32, i32* %.omp.iv, align 4
  %9 = load i32, i32* %.omp.ub, align 4
  %cmp4 = icmp sle i32 %8, %9
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, i32* %I, align 4
  %11 = load i32, i32* %I, align 4
  %12 = load i32*, i32** %A.addr, align 8
  %13 = load i32, i32* %I, align 4
  %idxprom = sext i32 %13 to i64
  %ptridx = getelementptr inbounds i32, i32* %12, i64 %idxprom
  store i32 %11, i32* %ptridx, align 4
  %14 = load i32, i32* %.omp.iv, align 4
  %add6 = add nsw i32 %14, 1
  store i32 %add6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.assume(i1 noundef)
