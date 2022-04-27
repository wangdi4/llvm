; RUN: llvm-as %s -o %t.bc
; RUN: %oclopt -prefetch %t.bc -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -prefetch -verify %t.bc -S -o %t.oll
; RUN: FileCheck %s --input-file=%t.oll

; ModuleID = 'Triad'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @Triad(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)*, float addrspace(1)** %0, align 8
  %2 = getelementptr i8, i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)*, float addrspace(1)** %3, align 8
  %5 = getelementptr i8, i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)*, float addrspace(1)** %6, align 8
  %8 = getelementptr i8, i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float*
  %10 = load float, float* %9, align 4
  %11 = getelementptr i8, i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8, i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to <{ [4 x i64] }>**
  %16 = load <{ [4 x i64] }>*, <{ [4 x i64] }>** %15, align 8
  %17 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %18 = load i64, i64* %17, align 8
  %19 = getelementptr <{ [4 x i64] }>, <{ [4 x i64] }>* %16, i64 0, i32 0, i64 0
  %20 = load i64, i64* %19, align 8
  %vector.size.i = ashr i64 %18, 4
  %21 = icmp eq i64 %vector.size.i, 0
  br i1 %21, label %_Triad_separated_args.exit, label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %entry
  %temp19vector_func.i = insertelement <16 x float> undef, float %10, i32 0
  %vector18vector_func.i = shufflevector <16 x float> %temp19vector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %22

; <label>:22                                      ; preds = %22, %dim_0_vector_pre_head.i
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 1) 
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 2)
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 1)
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 2)
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 5)
; CHECK: call void @llvm.x86.mic.prefetch(i8* %{{[a-zA-Z0123456789]+}}, i32 6)

  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %22 ]
  %dim_0_vector_tid.i = phi i64 [ %20, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %22 ]
  %extract.lhs.lhsvector_func.i = shl i64 %dim_0_vector_tid.i, 32
  %extractvector_func.i = ashr exact i64 %extract.lhs.lhsvector_func.i, 32
  %23 = getelementptr inbounds float, float addrspace(1)* %1, i64 %extractvector_func.i
  %ptrTypeCastvector_func.i = bitcast float addrspace(1)* %23 to <16 x float> addrspace(1)*
  %24 = load <16 x float>, <16 x float> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %25 = getelementptr inbounds float, float addrspace(1)* %4, i64 %extractvector_func.i
  %ptrTypeCast17vector_func.i = bitcast float addrspace(1)* %25 to <16 x float> addrspace(1)*
  %26 = load <16 x float>, <16 x float> addrspace(1)* %ptrTypeCast17vector_func.i, align 4
  %27 = fmul <16 x float> %26, %vector18vector_func.i
  %28 = fadd <16 x float> %24, %27
  %29 = getelementptr inbounds float, float addrspace(1)* %7, i64 %extractvector_func.i
  %ptrTypeCast20vector_func.i = bitcast float addrspace(1)* %29 to <16 x float> addrspace(1)*
  store <16 x float> %28, <16 x float> addrspace(1)* %ptrTypeCast20vector_func.i, align 4
  %dim_0_vector_inc_ind_var.i = add nuw nsw i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add nuw nsw i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %_Triad_separated_args.exit, label %22

_Triad_separated_args.exit:                      ; preds = %22, %entry.i
  ret void
}

; DEBUGIFY-NOT: WARNING
