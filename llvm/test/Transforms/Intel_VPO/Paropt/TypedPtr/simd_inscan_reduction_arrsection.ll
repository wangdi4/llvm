; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

;; static float x[10];
;; void array_inscan_reduction(float *B) {
;;   #pragma omp simd reduction(inscan,+:x[0:10])
;;   for (int i = 0; i < 3; i++) {
;;     x[5] += 1;
;;     #pragma omp scan inclusive(x[0:10])
;;     B[i] = x[5];
;;   }
;; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1x = internal global [10 x float] zeroinitializer, align 16

define void @_Z22array_inscan_reductionPf(float* %B) {
;
; CHECK-LABEL:  red.init.body:
; CHECK-NEXT:    [[RED_CPY_DEST_PTR0:%.*]] = phi float* [ [[_ZL1X_RED_GEP0:%.*]], [[DIR_OMP_SIMD_16_SPLIT13_SPLIT0:%.*]] ], [ [[RED_CPY_DEST_INC0:%.*]], [[RED_INIT_BODY0:%.*]] ]
; CHECK-NEXT:    [[RED_CPY_SRC_PTR0:%.*]] = phi float* [ getelementptr inbounds ([10 x float], [10 x float]* @_ZL1x, i32 0, i32 0), [[DIR_OMP_SIMD_16_SPLIT13_SPLIT0]] ], [ [[RED_CPY_SRC_INC0:%.*]], [[RED_INIT_BODY0]] ]
; CHECK-NEXT:    [[TMP3:%.*]] = load float, float* [[RED_CPY_SRC_PTR0]], align 4
; CHECK-NEXT:    store float [[TMP3]], float* [[RED_CPY_DEST_PTR0]], align 4
; CHECK-NEXT:    [[RED_CPY_DEST_INC0]] = getelementptr float, float* [[RED_CPY_DEST_PTR0]], i32 1
; CHECK-NEXT:    [[RED_CPY_SRC_INC0]] = getelementptr float, float* [[RED_CPY_SRC_PTR0]], i32 1
; CHECK-NEXT:    [[RED_CPY_DONE0:%.*]] = icmp eq float* [[RED_CPY_DEST_INC0]], [[TMP2:%.*]]
; CHECK-NEXT:    br i1 [[RED_CPY_DONE0]], label [[RED_INIT_DONE0:%.*]], label [[RED_INIT_BODY0]]

; CHECK-LABEL:  fastred.update.body:
; CHECK-NEXT:    [[FASTRED_CPY_DEST_PTR0:%.*]] = phi float* [ [[_ZL1X_FAST_RED_GEP_MINUS_OFFSET17_CAST_PLUS_OFFSET0:%.*]], [[OMP_INNER_FOR_COND_OMP_INNER_FOR_END_CRIT_EDGE0:%.*]] ], [ [[FASTRED_CPY_DEST_INC0:%.*]], [[FASTRED_UPDATE_BODY0:%.*]] ]
; CHECK-NEXT:    [[FASTRED_CPY_SRC_PTR0:%.*]] = phi float* [ [[_ZL1X_RED_GEP0]], [[OMP_INNER_FOR_COND_OMP_INNER_FOR_END_CRIT_EDGE0]] ], [ [[FASTRED_CPY_SRC_INC0:%.*]], [[FASTRED_UPDATE_BODY0]] ]
; CHECK-NEXT:    [[TMP20:%.*]] = load float, float* [[FASTRED_CPY_SRC_PTR0]], align 4
; CHECK-NEXT:    store float [[TMP20]], float* [[FASTRED_CPY_DEST_PTR0]], align 4
; CHECK-NEXT:    [[FASTRED_CPY_DEST_INC0]] = getelementptr float, float* [[FASTRED_CPY_DEST_PTR0]], i32 1
; CHECK-NEXT:    [[FASTRED_CPY_SRC_INC0]] = getelementptr float, float* [[FASTRED_CPY_SRC_PTR0]], i32 1
; CHECK-NEXT:    [[FASTRED_CPY_DONE0:%.*]] = icmp eq float* [[FASTRED_CPY_DEST_INC0]], [[TMP19:%.*]]
; CHECK-NEXT:    br i1 [[FASTRED_CPY_DONE0]], label [[FASTRED_UPDATE_DONE0:%.*]], label [[FASTRED_UPDATE_BODY0]]
;
entry:
  %B.addr = alloca float*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store float* %B, float** %B.addr, align 8
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0)
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1)
  store i32 2, i32* %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.INSCAN"([10 x float]* @_ZL1x, i64 1, i64 0, i64 10, i64 1, i64 1),
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
  %7 = load float, float* getelementptr inbounds ([10 x float], [10 x float]* @_ZL1x, i64 0, i64 5), align 4
  %add1 = fadd fast float %7, 1.000000e+00
  store float %add1, float* getelementptr inbounds ([10 x float], [10 x float]* @_ZL1x, i64 0, i64 5), align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
    "QUAL.OMP.INCLUSIVE"([10 x float]* @_ZL1x, i64 1) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SCAN"() ]
  %9 = load float, float* getelementptr inbounds ([10 x float], [10 x float]* @_ZL1x, i64 0, i64 5), align 4
  %10 = load float*, float** %B.addr, align 8
  %11 = load i32, i32* %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, float* %10, i64 %idxprom
  store float %9, float* %arrayidx, align 4
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12)
  %13 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %13, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  %14 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14)
  %15 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
