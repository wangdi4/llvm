; RUN: opt -passes='sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck  -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-prepare-args' -S %s | FileCheck %s

; This test checks that vectorized kernel's barrier special buffer size is not
; round up to multiple of VF if vectorized masked kernel doesn't exist.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(ptr addrspace(1) noalias %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !no_barrier_path !5 !kernel_has_sub_groups !5 !scalar_kernel !9 !vectorized_width !1 !kernel_execution_length !10 !kernel_has_barrier !11 !kernel_has_global_sync !5 !max_wg_dimensions !1 !barrier_buffer_size !12 !private_memory_size !12 !local_buffer_size !6 !arg_type_null_val !16 {
entry:
; CHECK-LABEL: @test
; CHECK: [[MUL01:%[0-9]+]] = mul nuw nsw i64 %InternalLocalSize_0, %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 8, %LocalSizeProd

  ret void
}

define void @__Vectorized_.test(ptr addrspace(1) noalias %dst, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !9 !no_barrier_path !5 !kernel_has_sub_groups !5 !scalar_kernel !13 !vectorized_width !14 !kernel_execution_length !15 !kernel_has_barrier !11 !kernel_has_global_sync !5 !max_wg_dimensions !1 !barrier_buffer_size !1 !private_memory_size !1 !local_buffer_size !6 !vectorization_dimension !6 !can_unite_workgroups !5 !arg_type_null_val !16 {
entry:
; CHECK-LABEL: @__Vectorized_.test
; CHECK: [[RELAXED_LS0:%.*]] = add nuw nsw i64 %InternalLocalSize_0, [[#VF_1:]]
; CHECK-NEXT: [[ROUNDUP:%.*]] = and i64 [[RELAXED_LS0]], -[[#VF_1+1]]
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 [[ROUNDUP]], %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 1, %LocalSizeProd

  ret void
}

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"dst"}
!8 = !{ptr @__Vectorized_.test}
!9 = !{null}
!10 = !{i32 5}
!11 = !{i1 true}
!12 = !{i32 8}
!13 = distinct !{null}
!14 = !{i32 16}
!15 = !{i32 6}
!16 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-40: WARNING: Instruction with empty DebugLoc in function test {{.*}}
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function __Vectorized_.test {{.*}}
; DEBUGIFY-NOT: WARNING
