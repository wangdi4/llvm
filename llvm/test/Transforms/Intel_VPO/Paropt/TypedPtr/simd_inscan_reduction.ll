; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; float foo(float *A, float *B, int N) {
;   float x = 1.0f;
; #pragma omp simd reduction(inscan, + : x)
;   for (int i=0; i<N; i++) {
;     x += A[i];
; #pragma omp scan inclusive(x)
;     B[i] = x;
;   }
;   return x;
; }

; CHECK-LABEL: entry:

; CHECK:         [[X_RED:%.*.red]] = alloca float, align
; CHECK:         [[X:%.*]] = alloca float, align

; CHECK:         [[X_LOAD:%.*]] = load float, float* [[X]]
; CHECK-NEXT:    store float [[X_LOAD]], float* [[X_RED]]

; CHECK-LABEL: call void @llvm.directive.region.exit(token {{%[0-9]+}}) [ "DIR.OMP.END.SIMD"() ]

; CHECK:         [[X_RED_LOAD:%.*]] = load float, float* [[X_RED]]
; CHECK-NEXT:    store float [[X_RED_LOAD]], float* [[X_FAST_RED:%.*]]

; CHECK:         [[X_FAST_RED_LOAD:%.*]] = load float, float* [[X_FAST_RED]]
; CHECK-NEXT:    store float [[X_FAST_RED_LOAD]], float* [[X]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @_Z3fooPfS_i(float* %A, float* %B, i32 %N) {
entry:
  %A.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %N.addr = alloca i32, align 4
  %x = alloca float, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store float* %B, float** %B.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = bitcast float* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  store float 1.000000e+00, float* %x, align 4
  %1 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  %2 = load i32, i32* %N.addr, align 4
  store i32 %2, i32* %.capture_expr.0, align 4
  %3 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3)
  %4 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %5 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %6 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6)
  %7 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7)
  %8 = load i32, i32* %.capture_expr.1, align 4
  store i32 %8, i32* %.omp.ub, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:INSCAN"(float* %x, i64 1),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]

  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %omp.precond.then
  %10 = load i32, i32* %.omp.iv, align 4
  %11 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12)
  %13 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %13, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4
  %14 = load float*, float** %A.addr, align 8
  %15 = load i32, i32* %i, align 4
  %idxprom = sext i32 %15 to i64
  %arrayidx = getelementptr inbounds float, float* %14, i64 %idxprom
  %16 = load float, float* %arrayidx, align 4
  %17 = load float, float* %x, align 4
  %add5 = fadd fast float %17, %16
  store float %add5, float* %x, align 4
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE"(float* %x, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.SCAN"() ]
  %19 = load float, float* %x, align 4
  %20 = load float*, float** %B.addr, align 8
  %21 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %21 to i64
  %arrayidx7 = getelementptr inbounds float, float* %20, i64 %idxprom6
  store float %19, float* %arrayidx7, align 4
  %22 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22)
  %23 = load i32, i32* %.omp.iv, align 4
  %add8 = add nsw i32 %23, 1
  store i32 %add8, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %24 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24)
  %25 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25)
  %26 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26)
  %27 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27)
  %28 = load float, float* %x, align 4
  %29 = bitcast float* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29)
  ret float %28
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

