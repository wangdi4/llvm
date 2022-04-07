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
  %num.vector.wi.i = and i64 %18, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %20
  %scalar.size.i = sub i64 %18, %num.vector.wi.i
  %21 = icmp eq i64 %vector.size.i, 0
  br i1 %21, label %scalarIf.i, label %__Triad_separated_args.exit

; CHECK-NOT: call void @llvm.x86.mic.prefetch

scalarIf.i:                                       ; preds = %24, %entry
  %22 = icmp eq i64 %18, %num.vector.wi.i
  br i1 %22, label %__Triad_separated_args.exit, label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %scalar_kernel_entry.i, %scalarIf.i
  %dim_0_ind_var.i = phi i64 [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ], [ 0, %scalarIf.i ]
  %dim_0_tid.i = phi i64 [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ], [ %max.vector.gid.i, %scalarIf.i ]
  %sext.i = shl i64 %dim_0_tid.i, 32
  %23 = ashr exact i64 %sext.i, 32
  %24 = getelementptr inbounds float, float addrspace(1)* %1, i64 %23
  %25 = load float, float addrspace(1)* %24, align 4
  %26 = getelementptr inbounds float, float addrspace(1)* %4, i64 %23
  %27 = load float , float addrspace(1)* %26, align 4
  %28 = fmul float %27, %10
  %29 = fadd float %25, %28
  %30 = getelementptr inbounds float, float addrspace(1)* %7, i64 %23
  store float %29, float addrspace(1)* %30, align 4
  %dim_0_inc_ind_var.i = add nuw nsw i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add nuw nsw i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %__Triad_separated_args.exit, label %scalar_kernel_entry.i

__Triad_separated_args.exit:                      ; preds = %scalarIf.i, %scalar_kernel_entry.i
  ret void
}

; DEBUGIFY-NOT: WARNING
