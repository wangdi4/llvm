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

; CHECK:         [[X_LOAD:%.*]] = load float, ptr [[X]]
; CHECK-NEXT:    store float [[X_LOAD]], ptr [[X_RED]]

; CHECK-LABEL: call void @llvm.directive.region.exit(token {{%[0-9]+}}) [ "DIR.OMP.END.SIMD"() ]

; CHECK:         [[X_RED_LOAD:%.*]] = load float, ptr [[X_RED]]
; CHECK-NEXT:    store float [[X_RED_LOAD]], ptr [[X_FAST_RED:%.*]]

; CHECK:         [[X_FAST_RED_LOAD:%.*]] = load float, ptr [[X_FAST_RED]]
; CHECK-NEXT:    store float [[X_FAST_RED_LOAD]], ptr [[X]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @_Z3fooPfS_i(ptr %A, ptr %B, i32 %N) {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %x = alloca float, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %x)
  store float 1.000000e+00, ptr %x, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0)
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.1)
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
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub)
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x, float 0.000000e+00, i32 1, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i)
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %8 = load ptr, ptr %A.addr, align 8
  %9 = load i32, ptr %i, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds float, ptr %8, i64 %idxprom
  %10 = load float, ptr %arrayidx, align 4
  %11 = load float, ptr %x, align 4
  %add5 = fadd fast float %11, %10
  store float %add5, ptr %x, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE:TYPED"(ptr %x, float 0.000000e+00, i32 1, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.SCAN"() ]
  %13 = load float, ptr %x, align 4
  %14 = load ptr, ptr %B.addr, align 8
  %15 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %15 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %14, i64 %idxprom6
  store float %13, ptr %arrayidx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4
  %add8 = add nsw i32 %16, 1
  store i32 %add8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.1)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0)
  %17 = load float, ptr %x, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr %x)
  ret float %17
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
