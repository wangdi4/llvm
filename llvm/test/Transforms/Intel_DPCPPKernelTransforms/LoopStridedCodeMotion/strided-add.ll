; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that
; 1. stride is computed from operands of add
; 2. phi is not created for %0. Its use is replaced with the first element extracted from strided phi of %1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test_fn(i64 %init.gid.dim0) {
dim_0_vector_pre_head:
; CHECK-LABEL: dim_0_vector_pre_head:
; CHECK: [[TRUNC:%[0-9]+]] = trunc i64 %init.gid.dim0 to i32
; CHECK-NEXT: %broadcast.splatinsertvector_func = insertelement <16 x i32> poison, i32 [[TRUNC]], i64 0
; CHECK-NEXT: %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> poison, <16 x i32> zeroinitializer
; CHECK-NEXT: [[ADD:%[0-9]+]] = add nuw <16 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>

  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %dim_0_vector_pre_head
; CHECK: [[PHI:%[0-9]+]] = phi <16 x i32> [ [[ADD]], %dim_0_vector_pre_head ], [ %strided.add, %entryvector_func ]
; CHECK: [[EE:%[0-9]+]] = extractelement <16 x i32> [[PHI]], i64 0
; CHECK: add nsw i32 0, [[EE]]
; CHECK: %strided.add = add nuw <16 x i32> [[PHI]], <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>

  %dim_0_vector_ind_var = phi i64 [ %init.gid.dim0, %dim_0_vector_pre_head ], [ %dim_0_vector_inc_ind_var, %entryvector_func ]
  %0 = trunc i64 %dim_0_vector_ind_var to i32
  %broadcast.splatinsertvector_func = insertelement <16 x i32> poison, i32 %0, i64 0
  %broadcast.splatvector_func = shufflevector <16 x i32> %broadcast.splatinsertvector_func, <16 x i32> poison, <16 x i32> zeroinitializer
  %1 = add nuw <16 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  %wide.insertvector_func = shufflevector <16 x i32> %1, <16 x i32> zeroinitializer, <32 x i32> <i32 0, i32 undef, i32 1, i32 undef, i32 2, i32 undef, i32 3, i32 undef, i32 4, i32 undef, i32 5, i32 undef, i32 6, i32 undef, i32 7, i32 undef, i32 8, i32 undef, i32 9, i32 undef, i32 10, i32 undef, i32 11, i32 undef, i32 12, i32 undef, i32 13, i32 undef, i32 14, i32 undef, i32 15, i32 undef>
  %2 = add nsw i32 0, %0
  %dim_0_vector_inc_ind_var = add nuw nsw i64 %dim_0_vector_ind_var, 16
  br label %entryvector_func
}

; DEBUGIFY-NOT: WARNING
