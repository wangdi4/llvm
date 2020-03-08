; RUN: opt -S < %s -VPlanDriver -vplan-print-after-vpentity-instrs -disable-output | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @linear_iv_test(i32* nocapture %k) {
; CHECK-LABEL:  After insertion VPEntities instructions:
; CHECK-NEXT:    [[BB0:BB[0-9]+]]:
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB1:BB[0-9]+]]
; CHECK-NEXT:    no PREDECESSORS
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]:
; CHECK-NEXT:     [DA: Divergent] i64 [[VP_INDVARS_IV_IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     [DA: Uniform]   i64 [[VP_INDVARS_IV_IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     [DA: Divergent] i32* [[ALLOCA:%.*]] = allocate-priv i32*
; CHECK-NEXT:     [DA: Uniform]   i32 [[LOAD1:%.*]] = load i32* [[FUNC_INP:%.*]]
; CHECK-NEXT:     [DA: Divergent] i32 [[I2_INIT:%.*]] = induction-init{add} i32 [[LOAD1]] i32 1
; CHECK-NEXT:     [DA: Divergent] store i32 [[I2_INIT:%.*]] i32* [[ALLOCA]]
; CHECK-NEXT:     [DA: Uniform]   i32 [[I2_STEP:%.*]] = induction-init-step{add} i32 1
  %sum.red = alloca double, align 8
  br label %simd.begin.region
simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() , "QUAL.OMP.REDUCTION.ADD"(double* %sum.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %k, i32 1)]
  br label %simd.loop
simd.loop:
  %index = phi i64 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %add = add i64 %index, 1
  %trunc1 = trunc i64 %add to i32
  store i32 %trunc1, i32* %k, align 4
  br label %simd.loop.exit
simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 2
  br i1 %vl.cond, label %simd.loop, label %simd.end.region
simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return
return:
  %retval = select i1 1, i32 3, i32 1
  ret i32 %retval
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
