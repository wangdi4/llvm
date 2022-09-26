;; This private is not safe due to subscript with more than 2 dimensions.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = hello_$Y.priv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__() {
alloca_0:
  %"hello_$K.linear.iv" = alloca i32, align 8
  %"hello_$Y.priv" = alloca [15 x [140 x [230 x float]]], align 16
  %"hello_$X46" = alloca [256 x float], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64, i8* }>, align 8
  %"var$545" = alloca [256 x i32], align 4
  %"var$545.sub" = getelementptr inbounds [256 x i32], [256 x i32]* %"var$545", i64 0, i64 0
  br label %bb6

bb6:                                              ; preds = %bb6, %alloca_0
  %"var$3.0" = phi i64 [ 1, %alloca_0 ], [ %add.1, %bb6 ]
  %"var$4.0" = phi i32 [ 1, %alloca_0 ], [ %add.2, %bb6 ]
  %"var$5[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$545.sub", i64 %"var$3.0")
  store i32 %"var$4.0", i32* %"var$5[]", align 1
  %add.1 = add nuw nsw i64 %"var$3.0", 1
  %add.2 = add nuw nsw i32 %"var$4.0", 1
  %exitcond60.not = icmp eq i64 %add.1, 257
  br i1 %exitcond60.not, label %loop_body9.preheader, label %bb6

loop_body9.preheader:                             ; preds = %bb6
  %"hello_$X46.sub" = getelementptr inbounds [256 x float], [256 x float]* %"hello_$X46", i64 0, i64 0
  br label %loop_body9

loop_body9:                                       ; preds = %loop_body9, %loop_body9.preheader
  %"$loop_ctr.053" = phi i64 [ %add.3, %loop_body9 ], [ 1, %loop_body9.preheader ]
  %"var$5[]3" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"var$545.sub", i64 %"$loop_ctr.053")
  %"var$5[]_fetch.7" = load i32, i32* %"var$5[]3", align 1
  %"(float)var$5[]_fetch.7$" = sitofp i32 %"var$5[]_fetch.7" to float
  %"hello_$X[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"hello_$X46.sub", i64 %"$loop_ctr.053")
  store float %"(float)var$5[]_fetch.7$", float* %"hello_$X[]", align 1
  %add.3 = add nuw nsw i64 %"$loop_ctr.053", 1
  %exitcond59.not = icmp eq i64 %add.3, 257
  br i1 %exitcond59.not, label %loop_exit10, label %loop_body9

loop_exit10:                                      ; preds = %loop_body9
  %.fca.0.gep30 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 0
  store i8 56, i8* %.fca.0.gep30, align 1
  %.fca.1.gep31 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 1
  store i8 4, i8* %.fca.1.gep31, align 1
  %.fca.2.gep32 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 2
  store i8 1, i8* %.fca.2.gep32, align 1
  %.fca.3.gep33 = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep33, align 1
  %"argblock.field_0$" = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %argblock, i64 0, i32 0
  store i64 10, i64* %"argblock.field_0$", align 8
  %"(float*)hello_$Y$" = getelementptr inbounds [15 x [140 x [230 x float]]], [15 x [140 x [230 x float]]]* %"hello_$Y.priv", i64 0, i64 0, i64 0, i64 0
  br label %bb11

bb11:                                             ; preds = %bb18, %loop_exit10
  %indvars.iv56 = phi i64 [ %indvars.iv.next57, %bb18 ], [ 1, %loop_exit10 ]
  %0 = trunc i64 %indvars.iv56 to i32
  %"(float)hello_$I_fetch.20$" = sitofp i32 %0 to float
  %"hello_$Y[]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 128800, float* nonnull elementtype(float) %"(float*)hello_$Y$", i64 %indvars.iv56)
  br label %DIR.OMP.SIMD.1

omp.pdo.body20:                                   ; preds = %DIR.OMP.SIMD.161, %omp.pdo.body20
  %omp.pdo.norm.iv.local.051 = phi i64 [ 0, %DIR.OMP.SIMD.161 ], [ %add.5, %omp.pdo.body20 ]
  %add.4 = shl i64 %omp.pdo.norm.iv.local.051, 32
  %sext = add i64 %add.4, 4294967296
  %int_sext9 = ashr exact i64 %sext, 32
  %"hello_$Y[][]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 920, float* nonnull elementtype(float) %"hello_$Y[]", i64 %int_sext9)
  %"hello_$Y[][][]" = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"hello_$Y[][]", i64 %indvars.iv)
  store float %"(float)hello_$I_fetch.20$", float* %"hello_$Y[][][]", align 1
  %add.5 = add nuw nsw i64 %omp.pdo.norm.iv.local.051, 1
  %exitcond.not = icmp eq i64 %add.5, 140
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.pdo.body20

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.END.SIMD.3, %bb11
  %indvars.iv = phi i64 [ %indvars.iv.next, %DIR.OMP.END.SIMD.3 ], [ 1, %bb11 ]
  br label %DIR.OMP.SIMD.161

DIR.OMP.SIMD.161:                                 ; preds = %DIR.OMP.SIMD.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"hello_$K.linear.iv", i32 1), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.PRIVATE"([15 x [140 x [230 x float]]]* %"hello_$Y.priv"), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.pdo.body20

DIR.OMP.END.SIMD.1:                               ; preds = %omp.pdo.body20
  store i32 140, i32* %"hello_$K.linear.iv", align 8
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond55.not = icmp eq i64 %indvars.iv.next, 231
  br i1 %exitcond55.not, label %bb18, label %DIR.OMP.SIMD.1

bb18:                                             ; preds = %DIR.OMP.END.SIMD.3
  %indvars.iv.next57 = add nuw nsw i64 %indvars.iv56, 1
  %exitcond58.not = icmp eq i64 %indvars.iv.next57, 16
  br i1 %exitcond58.not, label %bb14, label %bb11

bb14:                                             ; preds = %bb18
  ret void
}

declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
