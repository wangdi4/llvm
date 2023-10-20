; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%class.task_sequence = type { i32, i64 }

declare void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4), ptr, i64, i32, i32)

; CHECK: define internal void [[CREATE2:@_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_]](ptr addrspace(4) %0, ptr %1, i64 %2, i32 %3, i32 %4) {
; CHECK: %[[INVOKE2:.*]] = call ptr addrspace(4) [[CREATE2]].block_invoke_mapper
; CHECK: %[[CAST2:.*]] = addrspacecast ptr addrspace(4) %[[INVOKE2]] to ptr
; CHECK: store ptr %[[CAST2]], ptr %literal.invoke

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_()  {
entry:
  %ts = alloca %class.task_sequence, align 8
  %bc = addrspacecast ptr %ts to ptr addrspace(4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func1, i64 0, i32 10, i32 4)
; CHECK: call void [[CREATE1:@_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.[0-9]+]](ptr addrspace(4) %bc, ptr nonnull @func1, i64 0, i32 10, i32 4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func2, i64 0, i32 10, i32 4)
; CHECK: call void [[CREATE2]](ptr addrspace(4) %bc, ptr nonnull @func2, i64 0, i32 10, i32 4)
  ret void
}

define internal i32 @func1(i32 %a) {
entry:
  ret i32 %a
}

define internal i32 @func2(i32 %a) {
entry:
  ret i32 %a
}

; CHECK: define internal void [[CREATE1]]
; CHECK: %[[INVOKE1:.*]] = call ptr addrspace(4) [[CREATE1]].block_invoke_mapper
; CHECK: %[[CAST1:.*]] = addrspacecast ptr addrspace(4) %[[INVOKE1]] to ptr
; CHECK: store ptr %[[CAST1]], ptr %literal.invoke

; CHECK: define void @func2._block_invoke_kernel
; CHECK: call i32 @func2

; CHECK: define void @func1._block_invoke_kernel
; CHECK: call i32 @func1

; CHECK: define internal ptr addrspace(4) [[CREATE2]].block_invoke_mapper
; CHECK: ret ptr addrspace(4) addrspacecast (ptr @func2._block_invoke_kernel to ptr addrspace(4))

; CHECK: define internal ptr addrspace(4) [[CREATE1]].block_invoke_mapper
; CHECK: ret ptr addrspace(4) addrspacecast (ptr @func1._block_invoke_kernel to ptr addrspace(4))

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_}

; CHECK: !sycl.kernels = !{[[KERNELS:![0-9]+]]}
; CHECK: [[KERNELS]] =
; CHECK-SAME: ptr @func2._block_invoke_kernel
; CHECK-SAME: ptr @func1._block_invoke_kernel

; DEBUGIFY-NOT: WARNING: {{.*}} _ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_
