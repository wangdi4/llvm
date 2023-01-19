; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that WG loops are created in O0 mode in the case function
; call is not inlined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @foo(i32 addrspace(1)* noalias noundef %dst) #0 {
entry:
  %dst.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %call
  store i32 0, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #1

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(i32 addrspace(1)* noalias noundef align 4 %dst) #2 !kernel_arg_addr_space !2 !kernel_arg_access_qual !3 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 !kernel_arg_host_accessible !7 !kernel_arg_pipe_depth !8 !kernel_arg_pipe_io !5 !kernel_arg_buffer_location !5 !no_barrier_path !9 !kernel_has_sub_groups !7 !kernel_execution_length !10 !kernel_has_global_sync !7 !recommended_vector_length !2 {
entry:
; CHECK-LABEL: @test
; CHECK: %local.ids = alloca [3 x i64], align 8
; CHECK-NEXT: %local.id0 = getelementptr inbounds [3 x i64], [3 x i64]* %local.ids, i64 0, i32 0
; CHECK-NEXT: %local.id1 = getelementptr inbounds [3 x i64], [3 x i64]* %local.ids, i64 0, i32 1
; CHECK-NEXT: %local.id2 = getelementptr inbounds [3 x i64], [3 x i64]* %local.ids, i64 0, i32 2
; CHECK: %dim_0_sub_lid = sub nuw nsw i64 %base.gid.dim0, %base.gid.dim0
; CHECK-NEXT: %dim_1_sub_lid = sub nuw nsw i64 %base.gid.dim1, %base.gid.dim1
; CHECK-NEXT: %dim_2_sub_lid = sub nuw nsw i64 %base.gid.dim2, %base.gid.dim2
; CHECK: dim_2_pre_head:
; CHECK: dim_1_pre_head:
; CHECK: dim_0_pre_head:
; CHECK: scalar_kernel_entry:
; CHECK-NEXT: %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
; CHECK-NEXT: %dim_0_tid = phi i64 [ %dim_0_sub_lid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
; CHECK: store i64 %dim_0_tid, i64* %local.id0, align 8
; CHECK-NEXT: store i64 %dim_1_tid, i64* %local.id1, align 8
; CHECK-NEXT: store i64 %dim_2_tid, i64* %local.id2, align 8
; CHECK-NEXT: call void @foo(i32 addrspace(1)* noalias noundef {{.*}}, [3 x i64]* noalias %local.ids)

  %dst.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  call void @foo(i32 addrspace(1)* noundef %0) #4
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #3 = { convergent nounwind readnone willreturn }
attributes #4 = { convergent }

!opencl.compiler.options = !{!0}
!sycl.kernels = !{!1}

!0 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!1 = !{void (i32 addrspace(1)*)* @test}
!2 = !{i32 1}
!3 = !{!"none"}
!4 = !{!"int*"}
!5 = !{!""}
!6 = !{!"dst"}
!7 = !{i1 false}
!8 = !{i32 0}
!9 = !{i1 true}
!10 = !{i32 5}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim0, %local.size.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim1, %local.size.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @get_base_global_id.(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add i64 %base.gid.dim2, %local.size.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} sub nuw nsw i64 %base.gid.dim0, %base.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} sub nuw nsw i64 %base.gid.dim1, %base.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} sub nuw nsw i64 %base.gid.dim2, %base.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_2_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_0_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_0_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_1_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_1_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_2_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} add nuw nsw i64 %dim_2_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} br label %exit
; DEBUGIFY-NOT: WARNING
