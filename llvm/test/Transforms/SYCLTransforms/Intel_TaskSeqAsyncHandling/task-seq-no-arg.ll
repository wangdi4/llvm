; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; The test is to check if task function without argument is correctly handled.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::ext::intel::experimental::task_sequence" = type { i32, i64 }

declare void @_Z6task_av()

define void @_ZTS15ParallelForTask() {
entry:
  %ts_a.i = alloca %"class.sycl::_V1::ext::intel::experimental::task_sequence", align 8
  %ts_a.ascast.i = addrspacecast %"class.sycl::_V1::ext::intel::experimental::task_sequence"* %ts_a.i to %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*
  %id.i = getelementptr inbounds %"class.sycl::_V1::ext::intel::experimental::task_sequence", %"class.sycl::_V1::ext::intel::experimental::task_sequence"* %ts_a.i, i64 0, i32 1
  %0 = load i64, i64* %id.i, align 8
  call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_a.ascast.i, void ()* @_Z6task_av, i64 %0, i32 1)
  ret void
}

declare void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*, void ()*, i64, i32)

!sycl.kernels = !{!0}

!0 = !{void ()* @_ZTS15ParallelForTask}

; CHECK: define internal void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %{{.*}}, void ()* %{{.*}}, i64 %{{.*}}, i32 %{{.*}})
; CHECK: %block.invoke = call i8 addrspace(4)* @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli.block_invoke_mapper"(i8 addrspace(4)* %{{.*}})
; CHECK: call void @__async(i8 addrspace(4)* %{{.*}}, i32 %{{.*}}, i8 addrspace(4)* %block.invoke, i8 addrspace(4)* %{{.*}})

; CHECK: define void @_Z6task_av._block_invoke_kernel
; CHECK: call void @_Z6task_av()

; CHECK: define internal i8 addrspace(4)* @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli.block_invoke_mapper"(i8 addrspace(4)* %{{.*}})
; CHECK: ret i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8*)* @_Z6task_av._block_invoke_kernel to i8*) to i8 addrspace(4)*)

; Generated functions won't contain any debug info, so we only check debug info
; in the original kernel wasn't discarded.
; DEBUGIFY-NOT: WARNING: {{.*}} _ZTS15ParallelForTask
