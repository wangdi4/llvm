; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

;; static float x[20];
;; void array_inscan_reduction(float *B) {
;;   #pragma omp simd reduction(inscan,+:x[2:10])
;;   for (int i = 0; i < 3; i++) {
;;     x[5] += 1;
;;     #pragma omp scan inclusive(x[2:10])
;;     B[i] = x[5];
;;   }
;; }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1x = internal global [20 x float] zeroinitializer, align 16

define void @_Z22array_inscan_reductionPf(ptr %B) {
;
; CHECK:         [[ARRAY:@.*]] = internal global [20 x float] zeroinitializer, align 16
; CHECK:         "QUAL.OMP.REDUCTION.ADD:ARRSECT.INSCAN.TYPED"(ptr [[ARRAY]], float 0.000000e+00, i64 10, i64 2, i64 1)
;

entry:
  %B.addr = alloca ptr, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %B, ptr %B.addr, align 8
  %0 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0)
  %1 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %1)
  store i32 2, ptr %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.INSCAN.TYPED"(ptr @_ZL1x, float 0.000000e+00, i64 10, i64 2, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %5)
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %7 = load float, ptr getelementptr inbounds ([20 x float], ptr @_ZL1x, i64 0, i64 5), align 4
  %add1 = fadd fast float %7, 1.000000e+00
  store float %add1, ptr getelementptr inbounds ([20 x float], ptr @_ZL1x, i64 0, i64 5), align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE"(ptr @_ZL1x, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SCAN"() ]

  %9 = load float, ptr getelementptr inbounds ([20 x float], ptr @_ZL1x, i64 0, i64 5), align 4
  %10 = load ptr, ptr %B.addr, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, ptr %10, i64 %idxprom
  store float %9, ptr %arrayidx, align 4
  %12 = bitcast ptr %i to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %12)
  %13 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %13, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]

  %14 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %14)
  %15 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %15)
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
