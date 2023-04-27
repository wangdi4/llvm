; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that WG loops are created in O0 mode.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !7 !kernel_execution_length !10 !kernel_has_global_sync !7 !recommended_vector_length !2 {
entry:
; CHECK: %dst.addr = alloca ptr addrspace(1), align 8
; CHECK-NEXT: %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT: %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NEXT: %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
; CHECK-NEXT: %base.gid.dim1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT: %local.size.dim1 = call i64 @_Z14get_local_sizej(i32 1)
; CHECK-NEXT: %max.gid.dim1 = add i64 %base.gid.dim1, %local.size.dim1
; CHECK-NEXT: %base.gid.dim2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT: %local.size.dim2 = call i64 @_Z14get_local_sizej(i32 2)
; CHECK-NEXT: %max.gid.dim2 = add i64 %base.gid.dim2, %local.size.dim2
; CHECK: dim_2_pre_head:
; CHECK: dim_1_pre_head:
; CHECK: dim_0_pre_head:
; CHECK: scalar_kernel_entry:
; CHECK-NEXT: %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
; CHECK-NEXT: store ptr addrspace(1) %dst, ptr %dst.addr, align 8
; CHECK-NEXT: [[Load:%[0-9]+]] = load ptr addrspace(1), ptr %dst.addr, align 8
; CHECK-NEXT: %arrayidx = getelementptr inbounds i32, ptr addrspace(1) [[Load]], i64 %dim_0_ind_var
; CHECK-NEXT: store i32 0, ptr addrspace(1) %arrayidx, align 4
; CHECK-NEXT: %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
; CHECK-NEXT: %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; CHECK-NEXT: br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #2
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call
  store i32 0, ptr addrspace(1) %arrayidx, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #1

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nounwind readnone willreturn }

!opencl.compiler.options = !{!0}
!sycl.kernels = !{!1}

!0 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!1 = !{ptr @test}
!2 = !{i32 1}
!3 = !{!"none"}
!4 = !{!"int*"}
!5 = !{!""}
!6 = !{!"dst"}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{i1 true}
!10 = !{i32 7}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim0, %local.size.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim1, %local.size.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim2, %local.size.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_2_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_0_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_1_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_2_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %exit
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY-NOT: WARNING
