; RUN: opt -O3 -paropt=31 -S < %s | FileCheck %s
;
;void foo(float * A, float * B, int N) {
;#pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] = 2.0f * B[i];
;}
;
;void goo(float * A, float * B, int N) {
;#pragma omp parallel
;#pragma omp for
;   for (int i = 0; i < N; ++i)
;     A[i] = 2.0f * B[i];
;}
;
;void bar(float * A, int N) {
;#pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] = i;
;#pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] += i;
;}
;
;void car(float *A, float *B, int N) {
;  float C[2] = { 3.3f, 4.4f };
;  float D[2] = { 2.6f, 6.7f };
;
;#pragma omp parallel for
;  for(int i = 0; i < N-1; ++i)
;     A[i] += (B[i] * C[0]) + D[1];
;}
;
; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.car.C = private unnamed_addr constant [2 x float] [float 0x400A666660000000, float 0x40119999A0000000], align 4
@__const.car.D = private unnamed_addr constant [2 x float] [float 0x4004CCCCC0000000, float 0x401ACCCCC0000000], align 4

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

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 4, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, float*, i64, i64)* [[OUTLINED_FUNC:@.+]] to void (i32*, i32*, ...)*), float* %B, float* %A,

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

define dso_local void @goo(float* %A, float* %B, i32 %N) {
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

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, float*, i64)* [[OUTLINED_GOO:@.+]] to void (i32*, i32*, ...)*), float* %A, float* %B,

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(float** %A.addr), "QUAL.OMP.SHARED"(float** %B.addr), "QUAL.OMP.SHARED"(i32* %N.addr), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %1 = load i32, i32* %N.addr, align 4
  store i32 %1, i32* %.capture_expr., align 4
  %2 = load i32, i32* %.capture_expr., align 4
  %sub = sub nsw i32 %2, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4
  %3 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.capture_expr.1, align 4
  store i32 %4, i32* %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %6 = load i32, i32* %.omp.lb, align 4
  store i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32* %.omp.iv, align 4
  %8 = load i32, i32* %.omp.ub, align 4
  %cmp4 = icmp sle i32 %7, %8
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, i32* %i, align 4
  %10 = load float*, float** %B.addr, align 8
  %11 = load i32, i32* %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  %12 = load float, float* %arrayidx, align 4
  %mul6 = fmul float 2.000000e+00, %12
  %13 = load float*, float** %A.addr, align 8
  %14 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %14 to i64
  %arrayidx8 = getelementptr inbounds float, float* %13, i64 %idxprom7
  store float %mul6, float* %arrayidx8, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, i32* %.omp.iv, align 4
  %add9 = add nsw i32 %15, 1
  store i32 %add9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

define dso_local void @bar(float* %A, i32 %N) {
entry:
  %A.addr = alloca float*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp7 = alloca i32, align 4
  %.capture_expr.8 = alloca i32, align 4
  %.capture_expr.9 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i25 = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
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

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, i64, i64)* [[OUTLINED_BAR1:@.+]] to void (i32*, i32*, ...)*), float* %A,

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(float** %A.addr) ]
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
  %9 = load i32, i32* %i, align 4
  %conv = sitofp i32 %9 to float
  %10 = load float*, float** %A.addr, align 8
  %11 = load i32, i32* %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  store float %conv, float* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4
  %add6 = add nsw i32 %12, 1
  store i32 %add6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %13 = load i32, i32* %N.addr, align 4
  store i32 %13, i32* %.capture_expr.8, align 4
  %14 = load i32, i32* %.capture_expr.8, align 4
  %sub10 = sub nsw i32 %14, 0
  %sub11 = sub nsw i32 %sub10, 1
  %add12 = add nsw i32 %sub11, 1
  %div13 = sdiv i32 %add12, 1
  %sub14 = sub nsw i32 %div13, 1
  store i32 %sub14, i32* %.capture_expr.9, align 4
  %15 = load i32, i32* %.capture_expr.8, align 4
  %cmp15 = icmp slt i32 0, %15
  br i1 %cmp15, label %omp.precond.then17, label %omp.precond.end37

omp.precond.then17:                               ; preds = %omp.precond.end
  store i32 0, i32* %.omp.lb19, align 4
  %16 = load i32, i32* %.capture_expr.9, align 4
  store i32 %16, i32* %.omp.ub20, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, i64, i64)* [[OUTLINED_BAR2:@.+]] to void (i32*, i32*, ...)*), float* %A,

  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb19), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv18), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub20), "QUAL.OMP.PRIVATE"(i32* %i25), "QUAL.OMP.SHARED"(float** %A.addr) ]
  %18 = load i32, i32* %.omp.lb19, align 4
  store i32 %18, i32* %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc33, %omp.precond.then17
  %19 = load i32, i32* %.omp.iv18, align 4
  %20 = load i32, i32* %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %19, %20
  br i1 %cmp22, label %omp.inner.for.body24, label %omp.inner.for.end35

omp.inner.for.body24:                             ; preds = %omp.inner.for.cond21
  %21 = load i32, i32* %.omp.iv18, align 4
  %mul26 = mul nsw i32 %21, 1
  %add27 = add nsw i32 0, %mul26
  store i32 %add27, i32* %i25, align 4
  %22 = load i32, i32* %i25, align 4
  %conv28 = sitofp i32 %22 to float
  %23 = load float*, float** %A.addr, align 8
  %24 = load i32, i32* %i25, align 4
  %idxprom29 = sext i32 %24 to i64
  %arrayidx30 = getelementptr inbounds float, float* %23, i64 %idxprom29
  %25 = load float, float* %arrayidx30, align 4
  %add31 = fadd float %25, %conv28
  store float %add31, float* %arrayidx30, align 4
  br label %omp.body.continue32

omp.body.continue32:                              ; preds = %omp.inner.for.body24
  br label %omp.inner.for.inc33

omp.inner.for.inc33:                              ; preds = %omp.body.continue32
  %26 = load i32, i32* %.omp.iv18, align 4
  %add34 = add nsw i32 %26, 1
  store i32 %add34, i32* %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end35:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit36

omp.loop.exit36:                                  ; preds = %omp.inner.for.end35
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end37

omp.precond.end37:                                ; preds = %omp.loop.exit36, %omp.precond.end
  ret void
}

define dso_local void @car(float* %A, float* %B, i32 %N) {
entry:
  %A.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %N.addr = alloca i32, align 4
  %C = alloca [2 x float], align 4
  %D = alloca [2 x float], align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store float* %B, float** %B.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = bitcast [2 x float]* %C to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast ([2 x float]* @__const.car.C to i8*), i64 8, i1 false)
  %1 = bitcast [2 x float]* %D to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 bitcast ([2 x float]* @__const.car.D to i8*), i64 8, i1 false)
  %2 = load i32, i32* %N.addr, align 4
  %sub = sub nsw i32 %2, 1
  store i32 %sub, i32* %.capture_expr.0, align 4
  %3 = load i32, i32* %.capture_expr.0, align 4
  %sub1 = sub nsw i32 %3, 0
  %div = sdiv i32 %sub1, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %4 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %5 = load i32, i32* %.capture_expr.1, align 4
  store i32 %5, i32* %.omp.ub, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 6, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, float*, float*, i64, i64, i64, i64)* [[OUTLINED_CAR:@.+]] to void (i32*, i32*, ...)*), float* %A, float* %B, i64 4651317697086436147, i64 4672034252792424038,

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(float** %A.addr), "QUAL.OMP.SHARED"(float** %B.addr), "QUAL.OMP.SHARED"([2 x float]* %C), "QUAL.OMP.SHARED"([2 x float]* %D), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %7 = load i32, i32* %.omp.lb, align 4
  store i32 %7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %8 = load i32, i32* %.omp.iv, align 4
  %9 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %8, %9
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %11 = load float*, float** %B.addr, align 8
  %12 = load i32, i32* %i, align 4
  %idxprom = sext i32 %12 to i64
  %ptridx = getelementptr inbounds float, float* %11, i64 %idxprom
  %13 = load float, float* %ptridx, align 4
  %arrayidx = getelementptr inbounds [2 x float], [2 x float]* %C, i64 0, i64 0
  %14 = load float, float* %arrayidx, align 4
  %mul4 = fmul float %13, %14
  %arrayidx5 = getelementptr inbounds [2 x float], [2 x float]* %D, i64 0, i64 1
  %15 = load float, float* %arrayidx5, align 4
  %add6 = fadd float %mul4, %15
  %16 = load float*, float** %A.addr, align 8
  %17 = load i32, i32* %i, align 4
  %idxprom7 = sext i32 %17 to i64
  %ptridx8 = getelementptr inbounds float, float* %16, i64 %idxprom7
  %18 = load float, float* %ptridx8, align 4
  %add9 = fadd float %18, %add6
  store float %add9, float* %ptridx8, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32* %.omp.iv, align 4
  %add10 = add nsw i32 %19, 1
  store i32 %add10, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

; CHECK: define internal void [[OUTLINED_FUNC]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture readonly [[BBASE:%.+]], float* nocapture [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, float* [[BBASE]], i64 [[IV:%.+]]
; CHECK:   %{{.+}} = load float, float* [[BADDR]]
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]], i64 [[IV]]
; CHECK:   store float %{{.+}}, float* [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_GOO]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture [[ABASE:%.+]], float* nocapture readonly [[BBASE:%.+]], i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, float* [[BBASE]], i64 [[IV:%.+]]
; CHECK:   %{{.+}} = load float, float* [[BADDR]]
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]], i64 [[IV]]
; CHECK:   store float %{{.+}}, float* [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_BAR1]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]], i64 [[IV:%.+]]
; CHECK:   store float %{{.+}}, float* [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_BAR2]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]], i64 [[IV:%.+]]
; CHECK:   %{{.+}} = load float, float* [[AADDR]]
; CHECK:   store float %{{.+}}, float* [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_CAR]](i32* nocapture readonly %tid, i32* nocapture readnone %bid, float* nocapture [[ABASE:%.+]], float* nocapture readonly [[BBASE:%.+]], i64 [[CVAL:%.+]], i64 [[DVAL:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, float* [[BBASE]],
; CHECK:   [[BVAL:%.+]] = load float, float* [[BADDR]]
; CHECK:   [[MUL0:%.+]] = fmul float [[BVAL]], 0x400A666660000000
; CHECK:   [[ADD0:%.+]] = fadd float [[MUL0]], 0x401ACCCCC0000000
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, float* [[ABASE]],
; CHECK:   [[AVAL:%.+]] = load float, float* [[AADDR]]
; CHECK:   [[ADD1:%.+]] = fadd float [[AVAL]], [[ADD0]]
; CHECK:   store float [[ADD1]], float* [[AADDR]]
; CHECK: }
