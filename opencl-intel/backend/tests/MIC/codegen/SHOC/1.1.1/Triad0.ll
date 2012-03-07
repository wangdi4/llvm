; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__Triad_original(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, float) nounwind

declare i64 @get_global_id(i32) nounwind readnone

declare [7 x i64] @__WG.boundaries.Triad_original(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare void @__Triad_separated_args(float addrspace(1)* nocapture, float addrspace(1)* nocapture, float addrspace(1)* nocapture, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.Triad(float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

define void @Triad(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to float addrspace(1)**
  %1 = load float addrspace(1)** %0, align 8
  %2 = getelementptr i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to float addrspace(1)**
  %7 = load float addrspace(1)** %6, align 8
  %8 = getelementptr i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float*
  %10 = load float* %9, align 4
  %11 = getelementptr i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to <{ [4 x i64] }>**
  %16 = load <{ [4 x i64] }>** %15, align 8
  %17 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %18 = load i64* %17, align 8
  %19 = getelementptr <{ [4 x i64] }>* %16, i64 0, i32 0, i64 0
  %20 = load i64* %19, align 8
  %21 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 1
  %22 = load i64* %21, align 8
  %23 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 2
  %24 = load i64* %23, align 8
  %vector.size.i = ashr i64 %18, 4
  %num.vector.wi.i = and i64 %18, -16
  %max.vector.gid.i = add i64 %num.vector.wi.i, %20
  %scalar.size.i = sub i64 %18, %num.vector.wi.i
  %25 = icmp eq i64 %vector.size.i, 0
  br i1 %25, label %scalarIf.i, label %dim_2_vector_pre_head.i

dim_2_vector_pre_head.i:                          ; preds = %entry
  %tempvector_func.i = insertelement <16 x float> undef, float %10, i32 0
  %vectorvector_func.i = shufflevector <16 x float> %tempvector_func.i, <16 x float> undef, <16 x i32> zeroinitializer
  br label %dim_1_vector_pre_head.i

dim_1_vector_pre_head.i:                          ; preds = %dim_1_vector_exit.i, %dim_2_vector_pre_head.i
  %dim_2_vector_ind_var.i = phi i64 [ 0, %dim_2_vector_pre_head.i ], [ %dim_2_vector_inc_ind_var.i, %dim_1_vector_exit.i ]
  br label %dim_0_vector_pre_head.i

dim_0_vector_pre_head.i:                          ; preds = %dim_0_vector_exit.i, %dim_1_vector_pre_head.i
  %dim_1_vector_ind_var.i = phi i64 [ 0, %dim_1_vector_pre_head.i ], [ %dim_1_vector_inc_ind_var.i, %dim_0_vector_exit.i ]
  br label %entryvector_func.i

entryvector_func.i:                               ; preds = %entryvector_func.i, %dim_0_vector_pre_head.i
  %dim_0_vector_ind_var.i = phi i64 [ 0, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_ind_var.i, %entryvector_func.i ]
  %dim_0_vector_tid.i = phi i64 [ %20, %dim_0_vector_pre_head.i ], [ %dim_0_vector_inc_tid.i, %entryvector_func.i ]
  %extract.lhs.lhsvector_func.i = shl i64 %dim_0_vector_tid.i, 32
  %extractvector_func.i = ashr exact i64 %extract.lhs.lhsvector_func.i, 32
  %26 = getelementptr inbounds float addrspace(1)* %1, i64 %extractvector_func.i
  %ptrTypeCastvector_func.i = bitcast float addrspace(1)* %26 to <16 x float> addrspace(1)*
  %27 = load <16 x float> addrspace(1)* %ptrTypeCastvector_func.i, align 4
  %28 = getelementptr inbounds float addrspace(1)* %4, i64 %extractvector_func.i
  %ptrTypeCast18vector_func.i = bitcast float addrspace(1)* %28 to <16 x float> addrspace(1)*
  %29 = load <16 x float> addrspace(1)* %ptrTypeCast18vector_func.i, align 4
  %mul19vector_func.i = fmul <16 x float> %29, %vectorvector_func.i
  %add20vector_func.i = fadd <16 x float> %27, %mul19vector_func.i
  %30 = getelementptr inbounds float addrspace(1)* %7, i64 %extractvector_func.i
  %ptrTypeCast21vector_func.i = bitcast float addrspace(1)* %30 to <16 x float> addrspace(1)*
  store <16 x float> %add20vector_func.i, <16 x float> addrspace(1)* %ptrTypeCast21vector_func.i, align 4
  %dim_0_vector_inc_ind_var.i = add i64 %dim_0_vector_ind_var.i, 1
  %dim_0_vector_cmp.to.max.i = icmp eq i64 %dim_0_vector_inc_ind_var.i, %vector.size.i
  %dim_0_vector_inc_tid.i = add i64 %dim_0_vector_tid.i, 16
  br i1 %dim_0_vector_cmp.to.max.i, label %dim_0_vector_exit.i, label %entryvector_func.i

dim_0_vector_exit.i:                              ; preds = %entryvector_func.i
  %dim_1_vector_inc_ind_var.i = add i64 %dim_1_vector_ind_var.i, 1
  %dim_1_vector_cmp.to.max.i = icmp eq i64 %dim_1_vector_inc_ind_var.i, %22
  br i1 %dim_1_vector_cmp.to.max.i, label %dim_1_vector_exit.i, label %dim_0_vector_pre_head.i

dim_1_vector_exit.i:                              ; preds = %dim_0_vector_exit.i
  %dim_2_vector_inc_ind_var.i = add i64 %dim_2_vector_ind_var.i, 1
  %dim_2_vector_cmp.to.max.i = icmp eq i64 %dim_2_vector_inc_ind_var.i, %24
  br i1 %dim_2_vector_cmp.to.max.i, label %scalarIf.i, label %dim_1_vector_pre_head.i

scalarIf.i:                                       ; preds = %dim_1_vector_exit.i, %entry
  %31 = icmp eq i64 %18, %num.vector.wi.i
  br i1 %31, label %__Triad_separated_args.exit, label %dim_1_pre_head.i

dim_1_pre_head.i:                                 ; preds = %dim_1_exit.i, %scalarIf.i
  %dim_2_ind_var.i = phi i64 [ %dim_2_inc_ind_var.i, %dim_1_exit.i ], [ 0, %scalarIf.i ]
  br label %dim_0_pre_head.i

dim_0_pre_head.i:                                 ; preds = %dim_0_exit.i, %dim_1_pre_head.i
  %dim_1_ind_var.i = phi i64 [ 0, %dim_1_pre_head.i ], [ %dim_1_inc_ind_var.i, %dim_0_exit.i ]
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %scalar_kernel_entry.i, %dim_0_pre_head.i
  %dim_0_ind_var.i = phi i64 [ 0, %dim_0_pre_head.i ], [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ]
  %dim_0_tid.i = phi i64 [ %max.vector.gid.i, %dim_0_pre_head.i ], [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ]
  %sext.i = shl i64 %dim_0_tid.i, 32
  %idxprom.i = ashr exact i64 %sext.i, 32
  %arrayidx.i = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom.i
  %32 = load float addrspace(1)* %arrayidx.i, align 4
  %arrayidx2.i = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom.i
  %33 = load float addrspace(1)* %arrayidx2.i, align 4
  %mul.i = fmul float %33, %10
  %add.i = fadd float %32, %mul.i
  %arrayidx4.i = getelementptr inbounds float addrspace(1)* %7, i64 %idxprom.i
  store float %add.i, float addrspace(1)* %arrayidx4.i, align 4
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %scalar.size.i
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %dim_0_exit.i, label %scalar_kernel_entry.i

dim_0_exit.i:                                     ; preds = %scalar_kernel_entry.i
  %dim_1_inc_ind_var.i = add i64 %dim_1_ind_var.i, 1
  %dim_1_cmp.to.max.i = icmp eq i64 %dim_1_inc_ind_var.i, %22
  br i1 %dim_1_cmp.to.max.i, label %dim_1_exit.i, label %dim_0_pre_head.i

dim_1_exit.i:                                     ; preds = %dim_0_exit.i
  %dim_2_inc_ind_var.i = add i64 %dim_2_ind_var.i, 1
  %dim_2_cmp.to.max.i = icmp eq i64 %dim_2_inc_ind_var.i, %24
  br i1 %dim_2_cmp.to.max.i, label %__Triad_separated_args.exit, label %dim_1_pre_head.i

__Triad_separated_args.exit:                      ; preds = %scalarIf.i, %dim_1_exit.i
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}
!cl.noBarrierPath.kernels = !{!3}
!opencl.wrappers = !{!4}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, float, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__Triad_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{}
!3 = metadata !{metadata !"Triad"}
!4 = metadata !{void (i8*)* @Triad}
