; RUN: opt -passes=sycl-kernel-loop-strided-code-motion -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-loop-strided-code-motion -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that 'sitofp' instruction is not hoisted because the accurate result must be stored.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test() {
dim_0_vector_pre_head:
  br label %entryvector_func

entryvector_func:                                 ; preds = %entryvector_func, %dim_0_vector_pre_head
; CHECK-LABEL: entryvector_func:
; CHECK: [[TMP:%.*]] = phi <8 x i32> [ {{.*}}, %dim_0_vector_pre_head ], [ %strided.add, %entryvector_func ]
; CHECK: sitofp <8 x i32> [[TMP]] to <8 x float>
  %dim0__tid = phi i64 [ 0, %dim_0_vector_pre_head ], [ %dim0_inc_tid, %entryvector_func ]
  %0 = trunc i64 %dim0__tid to i32
  %broadcast.splatinsertvector_func = insertelement <8 x i32> zeroinitializer, i32 %0, i32 0
  %broadcast.splatvector_func = shufflevector <8 x i32> %broadcast.splatinsertvector_func, <8 x i32> zeroinitializer, <8 x i32> zeroinitializer
  %1 = add nuw <8 x i32> %broadcast.splatvector_func, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %convert.fp = sitofp <8 x i32> %1 to <8 x float>
  store <8 x float> %convert.fp, ptr addrspace(1) null, align 4
  %dim0_inc_tid = add nuw nsw i64 %dim0__tid, 8
  br label %entryvector_func
}

; DEBUGIFY-NOT: WARNING
