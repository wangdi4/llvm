; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; Check that function pointer is correctly replaced in select instruction.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_14kernel_handlerEE_(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define void @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_14kernel_handlerEE_() {
entry:
  %0 = call ptr addrspace(4) @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4cS5_i.block_invoke_mapper"()
  ret void
}

; CHECK: define void @_Z6my_sotPiS_i._block_invoke_kernel(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define void @_Z6my_sotPiS_i._block_invoke_kernel() {
  ret void
}

; CHECK: define void @_Z12produce_dataPiS_i._block_invoke_kernel(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)

define void @_Z12produce_dataPiS_i._block_invoke_kernel() {
  ret void
}

define internal ptr addrspace(4) @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4cS5_i.block_invoke_mapper"() {
entry:
; CHECK: define internal ptr addrspace(4) @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4cS5_i.block_invoke_mapper"(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle)
; CHECK: select i1 {{.*}}, ptr @_Z12produce_dataPiS_i._block_invoke_kernel, ptr @_Z6my_sotPiS_i._block_invoke_kernel

  %0 = select i1 false, ptr @_Z12produce_dataPiS_i._block_invoke_kernel, ptr @_Z6my_sotPiS_i._block_invoke_kernel
  ret ptr addrspace(4) null
}

; CHECK: !sycl.kernels = !{[[MD:![0-9]+]]}

!sycl.kernels = !{!0}

; CHECK: [[MD]] = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_14kernel_handlerEE_, ptr @_Z6my_sotPiS_i._block_invoke_kernel, ptr @_Z12produce_dataPiS_i._block_invoke_kernel

!0 = !{ptr @_ZTSZZ4mainENKUlRN4sycl3_V17handlerEE_clES2_EUlNS0_14kernel_handlerEE_, ptr @_Z6my_sotPiS_i._block_invoke_kernel, ptr @_Z12produce_dataPiS_i._block_invoke_kernel}

; DEBUGIFY-NOT: WARNING
