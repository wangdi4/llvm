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
; CHECK-NEXT:    [[Y_REF_ADDR_RED_REF:%.*.addr.red.ref]] = alloca ptr, align 8
; CHECK:         [[Y_REF_ADDR:%.*.addr]] = alloca ptr, align 8
; CHECK:         br

; CHECK:         [[Y_REF_ADDR_LOAD:%.*]] = load ptr, ptr [[Y_REF_ADDR]], align 8
; CHECK-NEXT:    [[Y_REF_ADDR_LOAD_LOAD:%.*]] = load float, ptr [[Y_REF_ADDR_LOAD]], align 4
; CHECK-NEXT:    store float [[Y_REF_ADDR_LOAD_LOAD]], ptr [[Y_REF_ADDR_RED]], align 4

; CHECK-LABEL:   call void @llvm.directive.region.exit

; CHECK:         [[Y_REF_ADDR_LOAD_2:%.*]] = load ptr, ptr [[Y_REF_ADDR]], align 8
; CHECK-NEXT:    [[Y_REF_ADDR_FAST_RED_LOAD:%.*]] = load float, ptr {{%.*.addr.fast_red}}, align 4
; CHECK-NEXT:    store float [[Y_REF_ADDR_FAST_RED_LOAD]], ptr [[Y_REF_ADDR_LOAD_2]], align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z29byref_inscan_reduction_simpleRfPf(ptr noundef nonnull align 4 dereferenceable(4) %y_ref, ptr noundef %B) {
entry:
  %y_ref.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %y_ref, ptr %y_ref.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub)
  store i32 2, ptr %.omp.ub, align 4
  %0 = load ptr, ptr %y_ref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF.INSCAN.TYPED"(ptr %y_ref.addr, float 0.000000e+00, i32 1, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i)
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load ptr, ptr %y_ref.addr, align 8
  %6 = load float, ptr %5, align 4
  %add1 = fadd fast float %6, 1.000000e+00
  store float %add1, ptr %5, align 4
  %7 = load ptr, ptr %y_ref.addr, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE:TYPED"(ptr %7, float 0.000000e+00, i32 1, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SCAN"() ]
  %9 = load ptr, ptr %y_ref.addr, align 8
  %10 = load float, ptr %9, align 4
  %11 = load ptr, ptr %B.addr, align 8
  %12 = load i32, ptr %i, align 4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds float, ptr %11, i64 %idxprom
  store float %10, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %13, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv)
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
