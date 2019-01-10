; RUN: opt -S -csa-fortran-intrinsics < %s | FileCheck %s

; ModuleID = 'fortran_intrinsics.f90'
source_filename = "fortran_intrinsics.f90"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i32 @g_(i32* %"g_$X") {
; CHECK: call void @llvm.csa.parallel.loop()
; CHECK: call void @llvm.csa.spmd(i32 5, i32 100)
; CHECK: call void @llvm.csa.pipeline.loop(i32 1)
alloca:
  %litaddr3 = alloca i32
  %litaddr2 = alloca i32
  %litaddr = alloca i32
  %"g_$X1" = alloca i32, align 8
  %"g_$G" = alloca i32, align 8
  %"var$1" = alloca [8 x i64], align 16
  br label %bb2

bb2:                                              ; preds = %alloca
  br label %bb3

bb3:                                              ; preds = %bb2
  br label %bb4

bb4:                                              ; preds = %bb3
  br label %bb7

bb7:                                              ; preds = %bb4
  br label %bb6

bb6:                                              ; preds = %bb7
  br label %bb8

bb8:                                              ; preds = %bb6
  call void @builtin_csa_parallel_loop_()
  br label %bb5

bb5:                                              ; preds = %bb8
  br label %bb9

bb9:                                              ; preds = %bb5
  store i32 5, i32* %litaddr
  store i32 100, i32* %litaddr2
  br label %bb10

bb10:                                             ; preds = %bb9
  br label %bb13

bb13:                                             ; preds = %bb10
  br label %bb12

bb12:                                             ; preds = %bb13
  br label %bb14

bb14:                                             ; preds = %bb12
  call void @builtin_csa_spmd_(i32* %litaddr, i32* %litaddr2)
  br label %bb11

bb11:                                             ; preds = %bb14
  br label %bb15

bb15:                                             ; preds = %bb11
  store i32 1, i32* %litaddr3
  br label %bb16

bb16:                                             ; preds = %bb15
  br label %bb19

bb19:                                             ; preds = %bb16
  br label %bb18

bb18:                                             ; preds = %bb19
  br label %bb20

bb20:                                             ; preds = %bb18
  call void @builtin_csa_pipeline_loop_(i32* %litaddr3)
  br label %bb17

bb17:                                             ; preds = %bb20
  br label %bb21

bb21:                                             ; preds = %bb17
  %"g_$X4" = load i32, i32* %"g_$X"
  store i32 %"g_$X4", i32* %"g_$G"
  br label %bb1

bb1:                                              ; preds = %bb21
  %"g_$G6" = load i32, i32* %"g_$G"
  ret i32 %"g_$G6"
}

declare void @builtin_csa_parallel_loop_()

declare void @builtin_csa_spmd_(i32*, i32*)

declare void @builtin_csa_pipeline_loop_(i32*)
