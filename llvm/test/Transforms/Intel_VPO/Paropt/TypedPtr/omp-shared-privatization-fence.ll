; RUN: opt -O3 -paropt=31 -S %s | FileCheck %s
;
; This test checks that presense of fence intructions inside the parallel region
; does not prevent privatization of a shared item that is not modified inside
; the region.
;
;void test_fence(float *A, int N) {
;#pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;#pragma omp critical
;     *A += 5.0;
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test_fence(float* %A, i32 %N) {
entry:
  %A.addr = alloca float*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = load i32, i32* %N.addr, align 4
  store i32 %0, i32* %.capture_expr.0, align 4
  %1 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %2 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %3 = load i32, i32* %.capture_expr.1, align 4
  store i32 %3, i32* %.omp.ub, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, i64, i64)* [[OUTLINED_TEST_FENCE:@.+]] to void (i32*, i32*, ...)*), float* %A,

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(float** %A.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %5 = load i32, i32* %.omp.lb, align 4
  store i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, i32* %.omp.iv, align 4
  %7 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  %10 = load float*, float** %A.addr, align 8
  %11 = load float, float* %10, align 4
  %conv = fpext float %11 to double
  %add5 = fadd fast double %conv, 5.000000e+00
  %conv6 = fptrunc double %add5 to float
  store float %conv6, float* %10, align 4
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CRITICAL"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4
  %add7 = add nsw i32 %12, 1
  store i32 %add7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

; CHECK: define internal void [[OUTLINED_TEST_FENCE]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture [[AADDR:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body:
; CHECK:   [[AVAL:%.+]] = load float, float* [[AADDR]]
; CHECK:   [[ADD:%.+]] = fadd fast float [[AVAL]], 5.000000e+00
; CHECK:   store float [[ADD]], float* [[AADDR]]
; CHECK: }
