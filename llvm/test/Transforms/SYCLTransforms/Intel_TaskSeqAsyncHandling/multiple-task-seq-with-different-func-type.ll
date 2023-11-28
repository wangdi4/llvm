; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; Builtin function type will not change if there is no sret arg.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.FunctionPacket = type <{ float, i8, [3 x i8] }>
%class.FixedVect = type { [2 x float] }
%"class.sycl::_V1::ext::intel::experimental::task_sequence" = type { i32, i64 }

define internal void @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE(ptr nocapture readonly byval(%class.FixedVect) align 4 %t2_array) {
  ret void
}

define internal void @_Z14function2ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEE9FixedVectIfLi2EEb(ptr addrspace(4) noalias nocapture writeonly sret(%class.FixedVect) align 4 %agg.result, i1 zeroext %shouldRead) {
  ret void
}

define internal void @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb(ptr addrspace(4) noalias nocapture writeonly sret(%struct.FunctionPacket) align 4 %agg.result, i1 zeroext %shouldRead) {
  ret void
}

define void @_ZTS16TaskSequenceTest() {
entry:
  %agg.tmp.i.i = alloca %class.FixedVect, align 4
  %ts_func1Array.i = alloca %"class.sycl::_V1::ext::intel::experimental::task_sequence", align 8
  %ts_func2Array.i = alloca %"class.sycl::_V1::ext::intel::experimental::task_sequence", align 8
  %ts_func2Struct.i = alloca %"class.sycl::_V1::ext::intel::experimental::task_sequence", align 8
  %ref.tmp.i = alloca %class.FixedVect, align 4
  %ts_func1Array.ascast.i = addrspacecast ptr %ts_func1Array.i to ptr addrspace(4)
  %ts_func2Array.ascast.i = addrspacecast ptr %ts_func2Array.i to ptr addrspace(4)
  %ts_func2Struct.ascast.i = addrspacecast ptr %ts_func2Struct.i to ptr addrspace(4)
  %ref.tmp.ascast.i = addrspacecast ptr %ref.tmp.i to ptr addrspace(4)
  %call.i.i = call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(ptr addrspace(4) %ts_func1Array.ascast.i, ptr nonnull @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE, i32 0)
; CHECK: call i64 [[CREATE:@"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"]](ptr addrspace(4) %ts_func1Array.ascast.i, ptr nonnull @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE, i32 0)
  %id.i.i = getelementptr inbounds %"class.sycl::_V1::ext::intel::experimental::task_sequence", ptr %ts_func1Array.i, i64 0, i32 1
  store i64 %call.i.i, ptr %id.i.i, align 8
  %call.i4.i = call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(ptr addrspace(4) %ts_func2Array.ascast.i, ptr nonnull @_Z14function2ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEE9FixedVectIfLi2EEb, i32 0)
; CHECK: call i64 [[CREATE]](ptr addrspace(4) %ts_func2Array.ascast.i, ptr nonnull @_Z14function2ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEE9FixedVectIfLi2EEb, i32 0)
  %id.i5.i = getelementptr inbounds %"class.sycl::_V1::ext::intel::experimental::task_sequence", ptr %ts_func2Array.i, i64 0, i32 1
  store i64 %call.i4.i, ptr %id.i5.i, align 8
  %call.i12.i = call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(ptr addrspace(4) %ts_func2Struct.ascast.i, ptr nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i32 0)
; CHECK: call i64 [[CREATE]](ptr addrspace(4) %ts_func2Struct.ascast.i, ptr nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i32 0)
  %id.i13.i = getelementptr inbounds %"class.sycl::_V1::ext::intel::experimental::task_sequence", ptr %ts_func2Struct.i, i64 0, i32 1
  store i64 %call.i12.i, ptr %id.i13.i, align 8
  call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliP15class.FixedVect"(ptr addrspace(4) %ts_func1Array.ascast.i, ptr nonnull @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE, i64 0, i32 0, ptr nonnull %agg.tmp.i.i)
; CHECK: call void [[ASYNC:@"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliP15class.FixedVect"]](ptr addrspace(4) %ts_func1Array.ascast.i, ptr nonnull @_Z14function1ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEEv9FixedVectIfLi2EE, i64 0, i32 0, ptr nonnull %agg.tmp.i.i)
  call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(ptr addrspace(4) %ts_func2Array.ascast.i, ptr nonnull @_Z14function2ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEE9FixedVectIfLi2EEb, i64 0, i32 0, i1 true)
; CHECK-NEXT: call void [[ASYNC1:@"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"]](ptr addrspace(4) %ts_func2Array.ascast.i, ptr nonnull @_Z14function2ArrayIN4sycl3_V13ext5intel4pipeI8my_pipe2fLi12EEEE9FixedVectIfLi2EEb, i64 0, i32 0, i1 true)
  call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(ptr addrspace(4) %ts_func2Struct.ascast.i, ptr nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 0, i32 0, i1 true)
; CHECK-NEXT: call void [[ASYNC1]](ptr addrspace(4) %ts_func2Struct.ascast.i, ptr nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 0, i32 0, i1 true)
  ret void
}

declare i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi"(ptr addrspace(4), ptr, i32)
; CHECK: define internal i64 [[CREATE]](ptr addrspace(4) %0, ptr %1, i32 %2) {
; CHECK-NEXT: [[PTRTOINT:%.*]] = ptrtoint ptr %1 to i64
; CHECK-NEXT: [[ICMP1:%.*]] = icmp eq i64 [[PTRTOINT]], ptrtoint (ptr @{{.*}} to i64)
; CHECK-NEXT: [[SELECT1:%.*]] = select i1 [[ICMP1]], i64 {{[0-9]+}}, i64 {{[0-9]+}}
; CHECK-NEXT: [[ICMP2:%.*]] = icmp eq i64 [[PTRTOINT]], ptrtoint (ptr @{{.*}} to i64)
; CHECK-NEXT: [[SELECT2:%.*]] = select i1 [[ICMP2]], i64 {{[0-9]+}}, i64 [[SELECT1]]
; CHECK-NEXT: [[PTR:%.*]] = call ptr @__create_task_sequence(i64 [[SELECT2]])
; CHECK-NEXT: [[RET:%.*]] = ptrtoint ptr [[PTR]] to i64
; CHECK-NEXT: ret i64 [[RET]]

declare void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEliP15class.FixedVect"(ptr addrspace(4), ptr, i64, i32, ptr)
; CHECK: define internal void [[ASYNC]]

declare void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(ptr addrspace(4), ptr, i64, i32, i1)
; CHECK: define internal void [[ASYNC1]]

declare void @"_Z32__spirv_TaskSequenceReleaseINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequence"(ptr addrspace(4))

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTS16TaskSequenceTest}

; DEBUGIFY-NOT: WARNING: {{.*}} _ZTS16TaskSequenceTest
