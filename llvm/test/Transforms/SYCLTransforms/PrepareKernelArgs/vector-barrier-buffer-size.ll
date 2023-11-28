; RUN: opt -passes='sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-prepare-args' -S %s | FileCheck %s

; This test checks that barrier special buffer size is rounded up to multiple of
; VF in vectorized masked kernel.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(ptr addrspace(1) noalias %src, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) local_unnamed_addr #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !8 !vectorized_masked_kernel !9 !no_barrier_path !5 !kernel_has_sub_groups !10 !opencl.stats.Vectorizer.CanVect !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !6 !vectorized_width !11 !vectorization_dimension !6 !scalar_kernel !12 !can_unite_workgroups !5 !kernel_execution_length !13 !kernel_has_barrier !10 !kernel_has_global_sync !5 !barrier_buffer_size !14 !private_memory_size !14 !local_buffer_size !6 !arg_type_null_val !16 {
FirstBB:
; CHECK-LABEL: @test
; CHECK: [[RELAXED_LS0:%.*]] = add nuw nsw i64 %InternalLocalSize_0, [[#VF_1:]]
; CHECK-NEXT: [[ROUNDUP:%.*]] = and i64 [[RELAXED_LS0]], -[[#VF_1+1]]
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 [[ROUNDUP]], %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 8, %LocalSizeProd

  ret void
}

define void @_ZGVeN16u_test(ptr addrspace(1) noalias %src, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) local_unnamed_addr #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !kernel_arg_name !7 !vectorized_kernel !12 !no_barrier_path !5 !kernel_has_sub_groups !10 !opencl.stats.Vectorizer.CanVect !1 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !6 !vectorized_width !11 !vectorization_dimension !6 !scalar_kernel !9 !can_unite_workgroups !5 !kernel_execution_length !15 !kernel_has_barrier !10 !kernel_has_global_sync !5 !barrier_buffer_size !1 !private_memory_size !1 !local_buffer_size !6 !recommended_vector_length !11 !arg_type_null_val !16 {
entry:
; CHECK-LABEL: @_ZGVeN16u_test
; CHECK: [[RELAXED_LS0:%.*]] = add nuw nsw i64 %InternalLocalSize_0, [[#VF_1:]]
; CHECK-NEXT: [[ROUNDUP:%.*]] = and i64 [[RELAXED_LS0]], -[[#VF_1+1]]
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i64 [[ROUNDUP]], %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i64 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i64 1, %LocalSizeProd

  ret void
}

attributes #0 = { nounwind willreturn }
attributes #1 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_test,_ZGVeM16u_test" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!4 = !{!""}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!"src"}
!8 = !{ptr @_ZGVeN16u_test}
!9 = distinct !{ptr @test}
!10 = !{i1 true}
!11 = !{i32 16}
!12 = !{null}
!13 = !{i32 7}
!14 = !{i32 8}
!15 = !{i32 10}
!16 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-40: WARNING: Instruction with empty DebugLoc in function test {{.*}}
; DEBUGIFY-COUNT-42: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}}
; DEBUGIFY-NOT: WARNING
