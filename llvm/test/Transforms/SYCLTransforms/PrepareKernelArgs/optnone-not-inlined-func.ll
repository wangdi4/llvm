; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY-TLS %s
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-prepare-args' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS

; This test checks that kernel with optnone are properly inlined into wrapper
; kernel.

; CHECK: define dso_local i32 @foo(
; CHECK: define dso_local void @test(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @foo(ptr addrspace(1) noalias noundef %dst, i32 noundef %v, i64 noundef %gid) #0 !kernel_arg_base_type !19 !arg_type_null_val !20 {
entry:
  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  %v.addr = alloca i32, align 4
  %gid.addr = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  store i32 %v, ptr %v.addr, align 4
  store i64 %gid, ptr %gid.addr, align 8
  %0 = load i32, ptr %v.addr, align 4
  %1 = load ptr addrspace(1), ptr %dst.addr, align 8
  %2 = load i64, ptr %gid.addr, align 8
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 %2
  store i32 %0, ptr addrspace(1) %arrayidx, align 4
  %3 = load i32, ptr %retval, align 4
  ret i32 %3
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst, i32 noundef %v) #1 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_name !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !no_barrier_path !12 !kernel_has_sub_groups !13 !barrier_buffer_size !14 !kernel_execution_length !15 !kernel_has_global_sync !13 !recommended_vector_length !16 !private_memory_size !17 !arg_type_null_val !18 {
  %dst.addr = alloca ptr addrspace(1), align 8
  %v.addr = alloca i32, align 4
  %gid = alloca i64, align 8
  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
  %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
  %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
  %base.gid.dim1 = call i64 @get_base_global_id.(i32 1)
  %local.size.dim1 = call i64 @_Z14get_local_sizej(i32 1)
  %max.gid.dim1 = add i64 %base.gid.dim1, %local.size.dim1
  %base.gid.dim2 = call i64 @get_base_global_id.(i32 2)
  %local.size.dim2 = call i64 @_Z14get_local_sizej(i32 2)
  %max.gid.dim2 = add i64 %base.gid.dim2, %local.size.dim2
  br label %dim_2_pre_head

dim_2_pre_head:                                   ; preds = %0
  br label %dim_1_pre_head

dim_1_pre_head:                                   ; preds = %dim_1_exit, %dim_2_pre_head
  %dim_2_ind_var = phi i64 [ %base.gid.dim2, %dim_2_pre_head ], [ %dim_2_inc_ind_var, %dim_1_exit ]
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %dim_0_exit, %dim_1_pre_head
  %dim_1_ind_var = phi i64 [ %base.gid.dim1, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
; CHECK-LABEL: scalar_kernel_entry
; CHECK-ARG: call i32 @foo(ptr addrspace(1) noalias noundef {{.*}}, i32 noundef {{.*}}, i64 noundef {{.*}}, ptr addrspace(3) noalias null, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias null, ptr noalias %RuntimeHandle)
; CHECK-TLS: call i32 @foo(ptr addrspace(1) noundef {{.*}}, i32 noundef {{.*}}, i64 noundef {{.*}})

  %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  store i32 %v, ptr %v.addr, align 4
  store i64 %dim_0_ind_var, ptr %gid, align 8
  %1 = load ptr addrspace(1), ptr %dst.addr, align 8
  %2 = load i32, ptr %v.addr, align 4
  %3 = load i64, ptr %gid, align 8
  %call1 = call i32 @foo(ptr addrspace(1) noundef %1, i32 noundef %2, i64 noundef %3) #3
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  %dim_1_inc_ind_var = add nuw nsw i64 %dim_1_ind_var, 1
  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head

dim_1_exit:                                       ; preds = %dim_0_exit
  %dim_2_inc_ind_var = add nuw nsw i64 %dim_2_ind_var, 1
  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
  br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head

dim_2_exit:                                       ; preds = %dim_1_exit
  br label %exit

exit:                                             ; preds = %dim_2_exit
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

declare i64 @get_base_global_id.(i32)

declare i64 @_Z14get_local_sizej(i32)

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{ptr @test}
!4 = !{i32 1, i32 0}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int"}
!7 = !{!"restrict", !""}
!8 = !{!"dst", !"v"}
!9 = !{i1 false, i1 false}
!10 = !{i32 0, i32 0}
!11 = !{!"", !""}
!12 = !{i1 true}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{i32 12}
!16 = !{i32 1}
!17 = !{i32 56}
!18 = !{ptr addrspace(1) null, i32 0}
!19 = !{!"int*", !"int", !"long"}
!20 = !{ptr addrspace(1) null, i32 0, i64 0}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-32: WARNING: Instruction with empty DebugLoc in function test {{.*}}
; DEBUGIFY-NOT: WARNING

; DEBUGIFY-TLS-NOT: WARNING
; DEBUGIFY-TLS-COUNT-36: WARNING: Instruction with empty DebugLoc in function test {{.*}}
; DEBUGIFY-TLS-NOT: WARNING
