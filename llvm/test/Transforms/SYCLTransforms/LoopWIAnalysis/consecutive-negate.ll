; RUN: opt -passes='print<sycl-kernel-loop-wi-analysis>' -S %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK:      Value dependencies:
; CHECK-NEXT:   STR    %dim0__tid = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
; CHECK-NEXT:   STR    %t = trunc i64 %dim0__tid to i32
; CHECK-NEXT:   RND    %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %t, i32 0
; CHECK-NEXT:   STR    %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK-NEXT:   STR    %a = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7>
; CHECK-NEXT:   STR    %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8
; CHECK-NEXT:   RND    %dim_0_vector_cmp.to.max = icmp eq i64 %dim0_inc_tid, %size
; CHECK-NEXT:   RND    br i1 %dim_0_vector_cmp.to.max, label %ret, label %entryvector_func
; CHECK-NEXT:   UNI  i64 8
; CHECK-NEXT: StrideIntermediate:
; CHECK-NEXT:   %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %t, i32 0
; CHECK-NEXT: ConstStrides:
; CHECK-NEXT:   i64 8    %dim0__tid = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
; CHECK-NEXT:   i32 8    %t = trunc i64 %dim0__tid to i32
; CHECK-NEXT:   i32 8    %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK-NEXT:   i32 8    %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %t, i32 0
; CHECK-NEXT:   i32 8    %a = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7>
; CHECK-NEXT:   i64 8    %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8

define dso_local void @test(i64 %size) {
dim_0_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %dim_0_vector_pre_head
  %dim0__tid = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
  %t = trunc i64 %dim0__tid to i32
  %broadcast.splatinsertvector_func = insertelement <8 x i32> poison, i32 %t, i32 0
  %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> poison, <8 x i32> zeroinitializer
  %a = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 -1, i32 -2, i32 -3, i32 -4, i32 -5, i32 -6, i32 -7>
  %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8
  %dim_0_vector_cmp.to.max = icmp eq i64 %dim0_inc_tid, %size
  br i1 %dim_0_vector_cmp.to.max, label %ret, label %entryvector_func

ret:                                              ; preds = %entryvector_func
  ret void
}
