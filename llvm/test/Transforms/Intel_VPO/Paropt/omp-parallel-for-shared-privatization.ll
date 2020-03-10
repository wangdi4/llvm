; RUN: opt -O3 -paropt=31 -S < %s | FileCheck %s
;
;void foo(float * A, float * B, int N) {
;#pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] = 2.0f * B[i];
;}
;
; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(float* %A, float* %B, i32 %N) {
entry:
  %A.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store float* %B, float** %B.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = load i32, i32* %N.addr, align 4
  store i32 %0, i32* %.capture_expr., align 4
  %1 = load i32, i32* %.capture_expr., align 4
  %sub = sub nsw i32 %1, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4
  %2 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %3 = load i32, i32* %.capture_expr.1, align 4
  store i32 %3, i32* %.omp.ub, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 4, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64, i64, i64, i64)* [[OUTLINED_FUNC:@.+]] to void (i32*, i32*, ...)*),

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(float** %B.addr), "QUAL.OMP.SHARED"(float** %A.addr) ]
  %5 = load i32, i32* %.omp.lb, align 4
  store i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, i32* %.omp.iv, align 4
  %7 = load i32, i32* %.omp.ub, align 4
  %cmp4 = icmp sle i32 %6, %7
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, i32* %i, align 4
  %9 = load float*, float** %B.addr, align 8
  %10 = load i32, i32* %i, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds float, float* %9, i64 %idxprom
  %11 = load float, float* %arrayidx, align 4
  %mul6 = fmul float 2.000000e+00, %11
  %12 = load float*, float** %A.addr, align 8
  %13 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %13 to i64
  %arrayidx8 = getelementptr inbounds float, float* %12, i64 %idxprom7
  store float %mul6, float* %arrayidx8, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4
  %add9 = add nsw i32 %14, 1
  store i32 %add9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; CHECK: define internal void [[OUTLINED_FUNC]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, i64 [[BPARAM:%.+]], i64 [[APARAM:%.+]], i64 %{{.+}}, i64 %{{.+}})
; CHECK-DAG:   [[BBASE:%.+]] = inttoptr i64 [[BPARAM]] to float*
; CHECK-DAG:   [[ABASE:%.+]] = inttoptr i64 [[APARAM]] to float*
; CHECK:     omp.inner.for.body:
; CHECK:       [[BADDR:%.+]] = getelementptr inbounds float, float* [[BBASE]], i64 [[IV:%.+]]
; CHECK:       %{{.+}} = load float, float* [[BADDR]]
; CHECK:       [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]], i64 [[IV]]
; CHECK:       store float %{{.+}}, float* [[AADDR]]

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
