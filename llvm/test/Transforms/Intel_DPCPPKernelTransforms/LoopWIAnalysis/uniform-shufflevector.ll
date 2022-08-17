; RUN: opt -passes='print<dpcpp-kernel-loop-wi-analysis>' -S %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -dpcpp-kernel-loop-wi-analysis -analyze -enable-new-pm=0 -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-DAG: STR    %dim_0_vector_inc_ind_var = add nuw nsw i32 %dim_0_vector_ind_var, 16
; CHECK-DAG: UNI  i32 16
; CHECK-DAG: STR    %dim_0_vector_ind_var = phi i32 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %pred.load.continue24vector_func ]
; CHECK-DAG: UNI    %broadcast.splatinsertvector_func = insertelement <16 x i32> zeroinitializer, i32 %a, i64 0
; CHECK-DAG: UNI    %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> zeroinitializer, <16 x i32> zeroinitializer
; CHECK-DAG: UNI  i32 %a
; CHECK-DAG: RND    br label %entryvector_func
; CHECK-DAG: RND    br label %pred.load.continue24vector_func

; CHECK-DAG: i32 16    %dim_0_vector_inc_ind_var = add nuw nsw i32 %dim_0_vector_ind_var, 16
; CHECK-DAG: i32 16    %dim_0_vector_ind_var = phi i32 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %pred.load.continue24vector_func ]

define void @test(i32 %a) {
dim_0_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %pred.load.continue24vector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i32 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %pred.load.continue24vector_func ]
  %broadcast.splatinsertvector_func = insertelement <16 x i32> zeroinitializer, i32 %a, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> zeroinitializer, <16 x i32> zeroinitializer
  br label %pred.load.continue24vector_func

pred.load.continue24vector_func:                  ; preds = %entryvector_func
  %dim_0_vector_inc_ind_var = add nuw nsw i32 %dim_0_vector_ind_var, 16
  br label %entryvector_func
}
