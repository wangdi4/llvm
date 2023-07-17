; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%class.task_sequence = type { i32, i64 }

declare void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4), ptr, i64, i32, i32)

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_()  {
entry:
  %ts = alloca %class.task_sequence, align 8
  %bc = addrspacecast ptr %ts to ptr addrspace(4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func1, i64 0, i32 10, i32 4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func2, i64 0, i32 10, i32 4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func3, i64 0, i32 10, i32 4)
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %bc, ptr nonnull @func4, i64 0, i32 10, i32 4)
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

define internal i32 @func3(i32 %a) {
entry:
  ret i32 %a
}

define internal i32 @func4(i32 %a) {
entry:
  ret i32 %a
}

; CHECK-LABEL: define internal void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_
; CHECK: %[[INVOKE:.*]] = call ptr addrspace(4) @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper
; CHECK: %[[CAST:.*]] = addrspacecast ptr addrspace(4) %[[INVOKE]] to ptr
; CHECK: store ptr %[[CAST]], ptr %literal.invoke

; CHECK-LABEL: define void @func4._block_invoke_kernel
; CHECK: call i32 @func4

; CHECK-LABEL: define void @func3._block_invoke_kernel
; CHECK: call i32 @func3

; CHECK-LABEL: define void @func2._block_invoke_kernel
; CHECK: call i32 @func2

; CHECK-LABEL: define void @func1._block_invoke_kernel
; CHECK: call i32 @func1

; CHECK-LABEL: define internal ptr addrspace(4) @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper
; CHECK-LABEL: entry:
; CHECK: [[TMP1:%.*]] = ptrtoint ptr addrspace(4) %0 to i64
; CHECK: [[TMP2:%.*]] = icmp eq i64 [[TMP1]], ptrtoint (ptr @func3 to i64)
; CHECK: [[TMP3:%.*]] = select i1 [[TMP2]], ptr @func3._block_invoke_kernel, ptr @func4._block_invoke_kernel
; CHECK: [[TMP4:%.*]] = icmp eq i64 [[TMP1]], ptrtoint (ptr @func2 to i64)
; CHECK: [[TMP5:%.*]] = select i1 [[TMP4]], ptr @func2._block_invoke_kernel, ptr [[TMP3]]
; CHECK: [[TMP6:%.*]] = icmp eq i64 [[TMP1]], ptrtoint (ptr @func1 to i64)
; CHECK: [[TMP7:%.*]] = select i1 [[TMP6]], ptr @func1._block_invoke_kernel, ptr [[TMP5]]
; CHECK: [[CAST:%.*]] = addrspacecast ptr [[TMP7]] to ptr addrspace(4)
; CHECK: ret ptr addrspace(4) [[CAST]]


!sycl.kernels = !{!0}

!0 = !{ptr @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_}

; CHECK: !sycl.kernels = !{[[KERNELS:![0-9]+]]}
; CHECK: [[KERNELS]] =
; CHECK-SAME: ptr @func4._block_invoke_kernel
; CHECK-SAME: ptr @func3._block_invoke_kernel
; CHECK-SAME: ptr @func2._block_invoke_kernel
; CHECK-SAME: ptr @func1._block_invoke_kernel

; DEBUGIFY-NOT: WARNING: {{.*}} _ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_
