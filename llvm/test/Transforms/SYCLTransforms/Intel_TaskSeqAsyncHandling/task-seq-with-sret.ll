; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; The test is to check if sret argument is lowered to return type.

; ModuleID = 'main'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.FunctionPacket = type <{ float, i8, [3 x i8] }>
%"class.sycl::_V1::ext::intel::experimental::task_sequence" = type { i32, i64 }

define void @_ZTS16TaskSequenceTest() {
entry:
  %ts_func2Struct.i = alloca %"class.sycl::_V1::ext::intel::experimental::task_sequence", align 8
  %ref.tmp.i = alloca %struct.FunctionPacket, align 4
  %ref.tmp.ascast.i = addrspacecast %struct.FunctionPacket* %ref.tmp.i to %struct.FunctionPacket addrspace(4)*
  %ts_func2Struct.ascast.i = addrspacecast %"class.sycl::_V1::ext::intel::experimental::task_sequence"* %ts_func2Struct.i to %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*
  %call.i15 = call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi.1"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, void (%struct.FunctionPacket addrspace(4)*, i1)* nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i32 0)
; CHECK: call i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi.1"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, %struct.FunctionPacket (i1)* @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i32 0)
  %id.i16 = getelementptr inbounds %"class.sycl::_V1::ext::intel::experimental::task_sequence", %"class.sycl::_V1::ext::intel::experimental::task_sequence"* %ts_func2Struct.i, i64 0, i32 1
  store i64 %call.i15, i64* %id.i16, align 8
  %0 = load i64, i64* %id.i16, align 8
  call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, void (%struct.FunctionPacket addrspace(4)*, i1)* nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 %0, i32 12, i1 true)
; CHECK: call void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, %struct.FunctionPacket (i1)* @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 {{.*}}, i32 12, i1 {{.*}})
  call void @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%struct.FunctionPacket addrspace(4)* sret(%struct.FunctionPacket) %ref.tmp.ascast.i, %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, void (%struct.FunctionPacket addrspace(4)*, i1)* nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 %0, i32 12)
; CHECK: call %struct.FunctionPacket @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, %struct.FunctionPacket (i1)* @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 {{.*}}, i32 12)
  call void @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%struct.FunctionPacket addrspace(4)* sret(%struct.FunctionPacket) %ref.tmp.ascast.i, %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, void (%struct.FunctionPacket addrspace(4)*, i1)* nonnull @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 %0, i32 12)
; CHECK: call %struct.FunctionPacket @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i, %struct.FunctionPacket (i1)* @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb, i64 {{.*}}, i32 12)
  call void @"_Z32__spirv_TaskSequenceReleaseINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequence"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* %ts_func2Struct.ascast.i)
  ret void
}

; CHECK: define internal %struct.FunctionPacket @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb(i1 zeroext %shouldRead)
; CHECK-NEXT: entry:
; CHECK-NEXT: %agg.result = alloca %struct.FunctionPacket, align 4, addrspace(4)
; CHECK: [[RESULT:%.*]] = load %struct.FunctionPacket, %struct.FunctionPacket addrspace(4)* %agg.result, align 1
; CHECK: ret %struct.FunctionPacket [[RESULT]]

define internal void @_Z15function2StructIN4sycl3_V13ext5intel4pipeI8my_pipe1fLi12EEEE14FunctionPacketb(%struct.FunctionPacket addrspace(4)* noalias nocapture writeonly sret(%struct.FunctionPacket) align 4 %agg.result, i1 zeroext %shouldRead) {
entry:
  ret void
}

declare void @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%struct.FunctionPacket addrspace(4)* sret(%struct.FunctionPacket), %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*, void (%struct.FunctionPacket addrspace(4)*, i1)*, i64, i32) local_unnamed_addr #4
; CHECK: define internal %struct.FunctionPacket @"_Z28__spirv_TaskSequenceGetINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEli"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* [[TASKSEQ:%.*]], %struct.FunctionPacket (i1)* {{.*}}, i64 {{.*}}, i32 [[LASTARG:%.*]])
; CHECK-NEXT: [[BC:%.*]] = bitcast %"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* [[TASKSEQ]] to i8 addrspace(4)*
; CHECK-NEXT: [[CALL:%.*]] = call i8 addrspace(4)* @__get(i8 addrspace(4)* [[BC]], i32 [[LASTARG]])
; CHECK-NEXT: [[CAST:%.*]] = addrspacecast i8 addrspace(4)* [[CALL]] to %struct.FunctionPacket*
; CHECK-NEXT: [[LOAD:%.*]] = load %struct.FunctionPacket, %struct.FunctionPacket* [[CAST]]
; CHECK-NEXT: ret %struct.FunctionPacket [[LOAD]]
; CHECK-NEXT: }

declare i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi.1"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*, void (%struct.FunctionPacket addrspace(4)*, i1)*, i32)
; CHECK: define internal i64 @"_Z31__spirv_TaskSequenceCreateINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvEi.1"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* {{.*}}, %struct.FunctionPacket (i1)* {{.*}}, i32 {{.*}})

declare void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*, void (%struct.FunctionPacket addrspace(4)*, i1)*, i64, i32, i1)
; CHECK: define internal void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequenceU13block_pointerFvvElib"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)* {{.*}}, %struct.FunctionPacket (i1)* {{.*}}, i64 {{.*}}, i32 {{.*}}, i1 {{.*}})

declare void @"_Z32__spirv_TaskSequenceReleaseINTELPU3AS456class.sycl::_V1::ext::intel::experimental::task_sequence"(%"class.sycl::_V1::ext::intel::experimental::task_sequence" addrspace(4)*)


!sycl.kernels = !{!0}

!0 = !{void ()* @_ZTS16TaskSequenceTest}


; Generated functions won't contain any debug info, so we only check debug info
; in the original kernel wasn't discarded.
; DEBUGIFY-NOT: WARNING: {{.*}} _ZTS16TaskSequenceTest
