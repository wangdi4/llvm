; RUN: opt -passes='sycl-kernel-prepare-args' -S %s | FileCheck %s
; RUN: opt -passes='sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that block_invoke kernel function is not broken by
; PrepareKernelArgs pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.ndrange_t.6 = type { i32, [3 x i64], [3 x i64], [3 x i64] }

define internal void @__block_fn_block_invoke(ptr addrspace(4) %.block_descriptor, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %0 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 5
  %RuntimeInterface4 = load ptr, ptr %0, align 1
  %1 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 6
  %Block2KernelMapper5 = load ptr, ptr %1, align 1
  %tmp.i2 = alloca %struct.ndrange_t.6, align 8
  %block.i = alloca <{ i32, i32, ptr addrspace(4), i64, i64 }>, align 8
  %2 = addrspacecast ptr %RuntimeInterface4 to ptr addrspace(4)
  %call1.i = tail call ptr @ocl20_get_default_queue(ptr addrspace(4) %2) #4
  %block.invoke.i = getelementptr inbounds <{ i32, i32, ptr addrspace(4), i64, i64 }>, ptr %block.i, i64 0, i32 2
  store ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke to ptr addrspace(4)), ptr %block.invoke.i, align 8
  %3 = addrspacecast ptr %block.i to ptr addrspace(4)
  %4 = addrspacecast ptr %RuntimeInterface4 to ptr addrspace(4)
  %5 = addrspacecast ptr %Block2KernelMapper5 to ptr addrspace(4)
  %6 = addrspacecast ptr %RuntimeHandle to ptr addrspace(4)
  %ndrange.ascast.i = addrspacecast ptr %tmp.i2 to ptr addrspace(4)

; CHECK-LABEL: @__block_fn_block_invoke(
; CHECK: call i32 @__ocl20_enqueue_kernel_basic
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke_kernel to ptr addrspace(4))

  %call2.i = call i32 @__ocl20_enqueue_kernel_basic(ptr %call1.i, i32 1, ptr addrspace(4) %ndrange.ascast.i, ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) %3, ptr addrspace(4) %4, ptr addrspace(4) %5, ptr addrspace(4) %6) #5

  ret void
}

; CHECK: define dso_local void @____block_fn_block_invoke_kernel_separated_args

define dso_local void @__block_fn_block_invoke_kernel(ptr addrspace(4) %0, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #1 !no_barrier_path !1 !kernel_has_sub_groups !1 !block_literal_size !2 !local_buffer_size !3 !barrier_buffer_size !4 !kernel_execution_length !5 !kernel_has_barrier !1 !kernel_has_global_sync !1 !private_memory_size !6 !opencl.stats.InstCounter.CantVectNonInlineUnsupportedFunctions !7 !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %1 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 5
  %RuntimeInterface9 = load ptr, ptr %1, align 1
  %2 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 6
  %Block2KernelMapper10 = load ptr, ptr %2, align 1
  %pSB_LocalId4 = alloca %struct.ndrange_t.6, align 8
  %3 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i64 0, i32 5
  %RuntimeInterface78 = load ptr, ptr %3, align 1
  %4 = addrspacecast ptr %RuntimeInterface78 to ptr addrspace(4)
  %call3.i = tail call ptr @ocl20_get_default_queue(ptr addrspace(4) %4) #4
  %5 = addrspacecast ptr %RuntimeInterface9 to ptr addrspace(4)
  %6 = addrspacecast ptr %Block2KernelMapper10 to ptr addrspace(4)
  %7 = addrspacecast ptr %RuntimeHandle to ptr addrspace(4)
  %ndrange.ascast.i = addrspacecast ptr %pSB_LocalId4 to ptr addrspace(4)
  %8 = getelementptr inbounds i8, ptr %pSpecialBuf, i64 40
  %block.invoke.i.i = getelementptr inbounds i8, ptr %pSpecialBuf, i64 48
  store ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke to ptr addrspace(4)), ptr %block.invoke.i.i, align 8
  %9 = addrspacecast ptr %8 to ptr addrspace(4)

; CHECK-LABEL: @__block_fn_block_invoke_kernel(
; CHECK: call i32 @__ocl20_enqueue_kernel_basic
; CHECK-SAME: ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke_kernel to ptr addrspace(4))

  %call4.i = call i32 @__ocl20_enqueue_kernel_basic(ptr %call3.i, i32 1, ptr addrspace(4) %ndrange.ascast.i, ptr addrspace(4) addrspacecast (ptr @__block_fn_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) %9, ptr addrspace(4) %5, ptr addrspace(4) %6, ptr addrspace(4) %7) #5

  ret void
}

declare ptr @ocl20_get_default_queue(ptr addrspace(4)) local_unnamed_addr #2

declare i32 @__ocl20_enqueue_kernel_basic(ptr, i32, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4)) local_unnamed_addr #3

attributes #0 = { convergent nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nounwind }
attributes #2 = { convergent nofree nounwind readnone willreturn mustprogress "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @__block_fn_block_invoke_kernel}
!1 = !{i1 false}
!2 = !{i32 32}
!3 = !{i32 0}
!4 = !{i32 112}
!5 = !{i32 29}
!6 = !{i32 128}
!7 = !{i32 1}
!8 = !{!"char*"}
!9 = !{ptr addrspace(4) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-40: WARNING: Instruction with empty DebugLoc in function __block_fn_block_invoke_kernel {{.*}}
; DEBUGIFY-NOT: WARNING
