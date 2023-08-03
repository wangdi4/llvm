; test enqueue_kernel with local memory arguments() are replaced with callbacks
; RUN: opt -passes=sycl-kernel-resolve-wi-call -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-resolve-wi-call -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@__block_literal_global = external addrspace(1) constant { i32, i32, ptr addrspace(4) }

declare i32 @__enqueue_kernel_varargs(ptr, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr)

declare i32 @__enqueue_kernel_events_varargs(ptr, i32, ptr, i32, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), i32, ptr)

define void @enqueue_block_with_local_arg(ptr addrspace(3) %pLocalMemBase, ptr %pWorkDim, ptr %pWGId, [4 x i64] %BaseGlbId, ptr %pSpecialBuf, ptr %RuntimeHandle) {
entry:
; CHECK-LABEL: define void @enqueue_block_with_local_arg(
; CHECK: call i32 @__ocl20_enqueue_kernel_localmem(ptr addrspace(1) %{{.*}}, i32 1, ptr addrspace(1) %{{.*}}, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_with_local_arg_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), i32 1, ptr %{{.*}}, ptr %RuntimeInterface, ptr %Block2KernelMapper, ptr %RuntimeHandle)
; CHECK-NOT: call i32 @__enqueue_kernel_vaargs(
; CHECK: call i32 @__ocl20_enqueue_kernel_events_localmem(ptr addrspace(1) %{{.*}}, i32 1, ptr addrspace(1) %{{.*}}, i32 0, ptr addrspacecast (ptr addrspace(4) null to ptr), ptr %{{.*}}, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_with_local_arg_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), i32 1, ptr %{{.*}}, ptr %RuntimeInterface, ptr %Block2KernelMapper, ptr %RuntimeHandle)
; CHECK-NOT: call i32 @__enqueue_kernel_events_vaargs(

  %0 = call i32 @__enqueue_kernel_varargs(ptr %pWorkDim, i32 1, ptr %pWorkDim, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_with_local_arg_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), i32 1, ptr %pWorkDim)
  %1 = addrspacecast ptr null to ptr addrspace(4)
  %2 = call i32 @__enqueue_kernel_events_varargs(ptr %pWorkDim, i32 1, ptr %pWorkDim, i32 0, ptr addrspace(4) null, ptr addrspace(4) %1, ptr addrspace(4) addrspacecast (ptr @__enqueue_block_with_local_arg_block_invoke_kernel to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @__block_literal_global to ptr addrspace(4)), i32 1, ptr %pWorkDim)
  ret void
}

define internal void @__enqueue_block_with_local_arg_block_invoke(ptr addrspace(4) %.block_descriptor, ptr addrspace(3) noundef %buf, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) {
entry:
  ret void
}

; Function Attrs: convergent
define void @__enqueue_block_with_local_arg_block_invoke_kernel(ptr addrspace(4) %0, ptr addrspace(3) %1, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 !arg_type_null_val !2 {
entry:
  ret void
}

attributes #0 = { convergent }

!opencl.ocl.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 3, i32 0}
!1 = !{ptr @enqueue_block_with_local_arg, ptr @__enqueue_block_with_local_arg_block_invoke_kernel}
!2 = !{ptr addrspace(4) null, ptr addrspace(3) null}

; DEBUGIFY-NOT: WARNING
