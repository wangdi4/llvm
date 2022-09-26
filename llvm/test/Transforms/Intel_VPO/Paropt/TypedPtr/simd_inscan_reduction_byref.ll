; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

;; void byref_inscan_reduction_simple(float (&y_ref), float *B) {
;;   #pragma omp simd reduction(inscan,+:y_ref)
;;   for (int i = 0; i < 3; i++) {
;;     y_ref += 1;
;;     #pragma omp scan inclusive(y_ref)
;;     B[i] = y_ref;
;;   }
;; }

; CHECK-LABEL: entry:

; CHECK:         [[Y_REF_ADDR_RED:%.*.addr.red]] = alloca float, align 8
; CHECK:         [[Y_REF_ADDR_RED_REF:%.*.addr.red.ref]] = alloca float*, align 8
; CHECK:         [[Y_REF_ADDR:%.*.addr]] = alloca float*, align 8

; CHECK:         [[Y_REF_ADDR_LOAD:%.*]] = load float*, float** [[Y_REF_ADDR]], align 8
; CHECK-NEXT:    [[Y_REF_ADDR_LOAD_LOAD:%.*]] = load float, float* [[Y_REF_ADDR_LOAD]], align 4
; CHECK-NEXT:    store float [[Y_REF_ADDR_LOAD_LOAD]], float* [[Y_REF_ADDR_RED]], align 4

; CHECK-LABEL:   call void @llvm.directive.region.exit

; CHECK:         [[Y_REF_ADDR_LOAD_2:%.*]] = load float*, float** [[Y_REF_ADDR]], align 8
; CHECK-NEXT:    [[Y_REF_ADDR_FAST_RED_LOAD:%.*]] = load float, float* {{%.*.addr.fast_red}}, align 4
; CHECK-NEXT:    store float [[Y_REF_ADDR_FAST_RED_LOAD]], float* [[Y_REF_ADDR_LOAD_2]], align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @_Z29byref_inscan_reduction_simpleRfPf(float* nonnull align 4 dereferenceable(4) %y_ref, float* %B) {
entry:
  %y_ref.addr = alloca float*, align 8
  %B.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %y_ref, float** %y_ref.addr, align 8
  store float* %B, float** %B.addr, align 8
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  store i32 2, i32* %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.INSCAN"(float** %y_ref.addr, i64 1),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.LINEAR:IV"(i32* %i, i32 1) ]

  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5)
  %6 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %7 = load float*, float** %y_ref.addr, align 8
  %8 = load float, float* %7, align 4
  %add1 = fadd fast float %8, 1.000000e+00
  store float %add1, float* %7, align 4
  %9 = load float*, float** %y_ref.addr, align 8
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE"(float* %9, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.SCAN"() ]
  %11 = load float*, float** %y_ref.addr, align 8
  %12 = load float, float* %11, align 4
  %13 = load float*, float** %B.addr, align 8
  %14 = load i32, i32* %i, align 4
  %idxprom = sext i32 %14 to i64
  %arrayidx = getelementptr inbounds float, float* %13, i64 %idxprom
  store float %12, float* %arrayidx, align 4
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15)
  %16 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %16, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  %17 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17)
  %18 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
