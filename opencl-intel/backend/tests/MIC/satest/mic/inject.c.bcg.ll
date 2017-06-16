; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__zero_imm_original(double addrspace(1)* nocapture, float addrspace(1)* nocapture, double addrspace(1)* nocapture, float addrspace(1)* nocapture) nounwind

declare i64 @_Z13get_global_idj(i32) nounwind readnone

declare [7 x i64] @__WG.boundaries.zero_imm_original(double addrspace(1)*, float addrspace(1)*, double addrspace(1)*, float addrspace(1)*)

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

declare void @__zero_imm_separated_args(double addrspace(1)* nocapture, float addrspace(1)* nocapture, double addrspace(1)* nocapture, float addrspace(1)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind alwaysinline

declare [7 x i64] @WG.boundaries.zero_imm(double addrspace(1)*, float addrspace(1)*, double addrspace(1)*, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

define void @zero_imm(i8* %pBuffer) {
entry:
  %0 = bitcast i8* %pBuffer to double addrspace(1)**
  %1 = load double addrspace(1)** %0, align 8
  %2 = getelementptr i8, i8* %pBuffer, i64 8
  %3 = bitcast i8* %2 to float addrspace(1)**
  %4 = load float addrspace(1)** %3, align 8
  %5 = getelementptr i8, i8* %pBuffer, i64 16
  %6 = bitcast i8* %5 to double addrspace(1)**
  %7 = load double addrspace(1)** %6, align 8
  %8 = getelementptr i8, i8* %pBuffer, i64 24
  %9 = bitcast i8* %8 to float addrspace(1)**
  %10 = load float addrspace(1)** %9, align 8
  %11 = getelementptr i8, i8* %pBuffer, i64 40
  %12 = bitcast i8* %11 to { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }**
  %13 = load { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }** %12, align 8
  %14 = getelementptr i8, i8* %pBuffer, i64 56
  %15 = bitcast i8* %14 to <{ [4 x i64] }>**
  %16 = load <{ [4 x i64] }>** %15, align 8
  %17 = getelementptr { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }* %13, i64 0, i32 3, i64 0
  %18 = load i64* %17, align 8
  %19 = getelementptr <{ [4 x i64] }>, <{ [4 x i64] }>* %16, i64 0, i32 0, i64 0
  %20 = load i64* %19, align 8
  br label %scalar_kernel_entry.i

scalar_kernel_entry.i:                            ; preds = %scalar_kernel_entry.i, %entry
  %dim_0_ind_var.i = phi i64 [ 0, %entry ], [ %dim_0_inc_ind_var.i, %scalar_kernel_entry.i ]
  %dim_0_tid.i = phi i64 [ %20, %entry ], [ %dim_0_inc_tid.i, %scalar_kernel_entry.i ]
  %sext.i = shl i64 %dim_0_tid.i, 32
  %idxprom.i = ashr exact i64 %sext.i, 32
  %arrayidx.i = getelementptr inbounds float, float addrspace(1)* %10, i64 %idxprom.i
  %21 = load float addrspace(1)* %arrayidx.i, align 4, !tbaa !11
  %sub.i = fadd float 0.000000e+00, %21
  %arrayidx2.i = getelementptr inbounds float, float addrspace(1)* %4, i64 %idxprom.i
  store float %sub.i, float addrspace(1)* %arrayidx2.i, align 4, !tbaa !11
  %arrayidx4.i = getelementptr inbounds double, double addrspace(1)* %7, i64 %idxprom.i
  %22 = load double addrspace(1)* %arrayidx4.i, align 8, !tbaa !14
  %sub5.i = fadd double 0.000000e+00, %22
  %arrayidx7.i = getelementptr inbounds double, double addrspace(1)* %1, i64 %idxprom.i
  store double %sub5.i, double addrspace(1)* %arrayidx7.i, align 8, !tbaa !14
  %dim_0_inc_ind_var.i = add i64 %dim_0_ind_var.i, 1
  %dim_0_cmp.to.max.i = icmp eq i64 %dim_0_inc_ind_var.i, %18
  %dim_0_inc_tid.i = add i64 %dim_0_tid.i, 1
  br i1 %dim_0_cmp.to.max.i, label %__zero_imm_separated_args.exit, label %scalar_kernel_entry.i

__zero_imm_separated_args.exit:                   ; preds = %scalar_kernel_entry.i
  ret void
}

!0 = !{void ()* @thisIsKernel, !1}
!1 = !{!"kernel_wrapper", void ()* @thisIsKernel}

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!8}
!opencl.kernel_info = !{!15}

!0 = !{void (double addrspace(1)*, float addrspace(1)*, double addrspace(1)*, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__zero_imm_separated_args, !1, !2}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = !{!"cl_kernel_arg_info", !3, !4, !5, !6, !7}
!3 = !{i32 0, i32 0, i32 0, i32 0}
!4 = !{i32 3, i32 3, i32 3, i32 3}
!5 = !{!"double*", !"float*", !"double*", !"float*"}
!6 = !{i32 0, i32 0, i32 1, i32 1}
!7 = !{!"outD", !"outF", !"inD", !"inF"}
!8 = !{!"-cl-std=CL1.2", !"-cl-kernel-arg-info"}
!9 = !{!"no_barrier_path", i1 true}
!10 = !{!"kernel_wrapper", void (i8*)* @zero_imm}
!11 = !{!"float", !12}
!12 = !{!"omnipotent char", !13}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!"double", !12}

!15 = !{void (double addrspace(1)*, float addrspace(1)*, double addrspace(1)*, float addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__zero_imm_separated_args, !16}
!16 = !{!9, !10}
