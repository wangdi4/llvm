;; This subscript is safe for SOA transform.
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOASafe = hello_$X.priv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2

define void @MAIN__() {
DIR.OMP.SIMD.119:
  %"hello_$I.linear.iv" = alloca i32, align 8
  %"hello_$X.priv" = alloca [256 x float], align 16
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.119
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"hello_$I.linear.iv", i32 1), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.PRIVATE"([256 x float]* %"hello_$X.priv"), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %"(float*)hello_$X$" = getelementptr inbounds [256 x float], [256 x float]* %"hello_$X.priv", i64 0, i64 0
  br label %omp.pdo.body8

omp.pdo.body8:                                    ; preds = %DIR.OMP.SIMD.2, %omp.pdo.body8
  %omp.pdo.norm.iv.local.016 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add.2, %omp.pdo.body8 ]
  %int_sext = trunc i64 %omp.pdo.norm.iv.local.016 to i32
  %add.1 = add nsw i32 %int_sext, 1
  %"(float)hello_$I_fetch.10$" = sitofp i32 %add.1 to float
  %int_sext3 = sext i32 %add.1 to i64
  %"hello_$X[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"(float*)hello_$X$", i64 %int_sext3)
  store float %"(float)hello_$I_fetch.10$", float* %"hello_$X[]", align 1
  %add.2 = add nuw nsw i64 %omp.pdo.norm.iv.local.016, 1
  %exitcond.not = icmp eq i64 %add.2, 256
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.220, label %omp.pdo.body8

DIR.OMP.END.SIMD.220:                             ; preds = %omp.pdo.body8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.220
  ret void
}

declare i32 @for_set_fpe_(i32* nocapture readonly)

declare i32 @for_set_reentrancy(i32* nocapture readonly)

declare token @llvm.directive.region.entry()

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

declare void @llvm.directive.region.exit(token)
