; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that stride is computed from operands of icmp.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() {
; CHECK-LABEL: @test
; CHECK: [[STRIDED:%[0-9]+]] = phi <16 x i32> [ {{.*}} ], [ %strided.add
; CHECK: icmp eq <16 x i32> [[STRIDED]], <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>

dim_0_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %pred.load.continue24vector_func, %dim_0_vector_pre_head
  %dim_0_vector_ind_var = phi i32 [ 0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %pred.load.continue24vector_func ]
  %broadcast.splatinsertvector_func = insertelement <16 x i32> zeroinitializer, i32 %dim_0_vector_ind_var, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> zeroinitializer, <16 x i32> zeroinitializer
  %0 = icmp eq <16 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>
  br label %pred.load.continue24vector_func

pred.load.continue24vector_func:                  ; preds = %entryvector_func
  %dim_0_vector_inc_ind_var = add nuw nsw i32 %dim_0_vector_ind_var, 16
  br label %entryvector_func
}

define void @trunc() {
; CHECK-LABEL: @trunc
; CHECK: [[STRIDED:%[0-9]+]] = phi <16 x i32> [ {{.*}} ], [ %strided.add
; CHECK: icmp eq <16 x i32> [[STRIDED]], <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>

dim_1_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %pred.load.continue28vector_func, %dim_1_vector_pre_head
  %dim_1_vector_ind_var = phi i64 [ 0, %dim_1_vector_pre_head ], [ %dim_1_vector_inc_ind_var, %pred.load.continue28vector_func ]
  %0 = trunc i64 %dim_1_vector_ind_var to i32
  %broadcast.splatinsertvector_func = insertelement <16 x i32> zeroinitializer, i32 %0, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> zeroinitializer, <16 x i32> zeroinitializer
  %1 = icmp eq <16 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>
  br label %pred.load.continue28vector_func

pred.load.continue28vector_func:                  ; preds = %entryvector_func
  %dim_1_vector_inc_ind_var = add nuw nsw i64 %dim_1_vector_ind_var, 16
  br label %entryvector_func
}

define void @icmp_has_user() {
; CHECK-LABEL: @icmp_has_user
; CHECK: [[STRIDED:%[0-9]+]] = phi <16 x i32> [ {{.*}} ], [ %strided.add
; CHECK: icmp eq <16 x i32> [[STRIDED]], <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>

dim_1_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %pred.load.continue28vector_func, %dim_1_vector_pre_head
  %dim_1_vector_ind_var = phi i64 [ 0, %dim_1_vector_pre_head ], [ %dim_1_vector_inc_ind_var, %pred.load.continue28vector_func ]
  %0 = trunc i64 %dim_1_vector_ind_var to i32
  %broadcast.splatinsertvector_func = insertelement <16 x i32> zeroinitializer, i32 %0, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> zeroinitializer, <16 x i32> zeroinitializer
  %1 = icmp eq <16 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>
  %2 = bitcast <16 x i1> %1 to i16
  br label %pred.load.continue28vector_func

pred.load.continue28vector_func:                  ; preds = %entryvector_func
  %dim_1_vector_inc_ind_var = add nuw nsw i64 %dim_1_vector_ind_var, 16
  br label %entryvector_func
}

; Strided IV has non-hoisted user other than icmp.

define void @non_hoisted_user() {
; CHECK-LABEL: @non_hoisted_user
; CHECK: [[STRIDED:%[0-9]+]] = phi <16 x i32> [ {{.*}} ], [ %strided.add
; CHECK: [[EXT:%[0-9]+]] = extractelement <16 x i32> [[STRIDED]], i64 0
; CHECK: add nsw i32 [[EXT]]
; CHECK: icmp eq <16 x i32> [[STRIDED]], <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>

dim_1_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %pred.load.continue28vector_func, %dim_1_vector_pre_head
  %dim_1_vector_ind_var = phi i64 [ 0, %dim_1_vector_pre_head ], [ %dim_1_vector_inc_ind_var, %pred.load.continue28vector_func ]
  %0 = trunc i64 %dim_1_vector_ind_var to i32
  %broadcast.splatinsertvector_func = insertelement <16 x i32> poison, i32 %0, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> poison, <16 x i32> zeroinitializer
  %1 = add nsw i32 %0, 0
  %2 = icmp eq <16 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7, i32 -8, i32 -9, i32 -10, i32 -11, i32 -12, i32 -13, i32 -14, i32 -15>
  br label %pred.load.continue28vector_func

pred.load.continue28vector_func:                  ; preds = %entryvector_func
  %dim_1_vector_inc_ind_var = add nuw nsw i64 %dim_1_vector_ind_var, 16
  br label %entryvector_func
}

; DEBUGIFY-NOT: WARNING
