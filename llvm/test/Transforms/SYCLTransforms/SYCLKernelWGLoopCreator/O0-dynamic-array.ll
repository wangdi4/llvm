; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that WG loops are created in O0 mode with dynamic array.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @reduction_test() !no_barrier_path !1 {
entry:
; CHECK: %COUNTER_N0.8.red.ascast.alloc.num_elements = udiv i64 0, 4
; CHECK-NEXT: %COUNTER_N0.8.red.ascast.data = alloca i32, i64 %COUNTER_N0.8.red.ascast.alloc.num_elements, align 4
; CHECK: dim_2_pre_head:
; CHECK: dim_1_pre_head:
; CHECK: dim_0_pre_head:
; CHECK: scalar_kernel_entry:
; CHECK-NEXT: %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
; CHECK-NEXT: %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
; CHECK-NEXT: %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; CHECK-NEXT: br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

  %COUNTER_N0.8.red.ascast.alloc.num_elements = udiv i64 0, 4
  %COUNTER_N0.8.red.ascast.data = alloca i32, i64 %COUNTER_N0.8.red.ascast.alloc.num_elements, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @reduction_test}
!1 = !{i1 true}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %base.gid.dim1 = call i64 @get_base_global_id.(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %local.size.dim1 = call i64 @_Z14get_local_sizej(i32 1)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %max.gid.dim1 = add i64 %base.gid.dim1, %local.size.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %base.gid.dim2 = call i64 @get_base_global_id.(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %local.size.dim2 = call i64 @_Z14get_local_sizej(i32 2)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %max.gid.dim2 = add i64 %base.gid.dim2, %local.size.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_0_sub_lid = sub nuw nsw i64 %base.gid.dim0, %base.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_1_sub_lid = sub nuw nsw i64 %base.gid.dim1, %base.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_2_sub_lid = sub nuw nsw i64 %base.gid.dim2, %base.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br label %dim_2_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  store i64 %dim_2_tid, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2), align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  store i64 %dim_1_tid, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1), align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_1_inc_ind_var = add nuw nsw i64 %dim_1_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_1_inc_tid = add nuw nsw i64 %dim_1_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_2_inc_ind_var = add nuw nsw i64 %dim_2_ind_var, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  %dim_2_inc_tid = add nuw nsw i64 %dim_2_tid, 1
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function reduction_test --  br label %exit
; DEBUGIFY-NOT: WARNING
