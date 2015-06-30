; test get_default_queue() and enqueue_kernel() are replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"

%struct.__block_descriptor = type { i64, i64 }
%opencl.queue_t = type opaque
%opencl.ndrange_t = type opaque

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [6 x i8] c"v8@?0\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([6 x i8]* @.str, i32 0, i32 0), i8* null }
@__block_literal_global = internal constant { i8**, i32, i32, i8*, %struct.__block_descriptor* } { i8** @_NSConcreteGlobalBlock, i32 1342177280, i32 0, i8* bitcast (void (i8*)* @__enqueue_simple_block_block_invoke to i8*), %struct.__block_descriptor* bitcast ({ i64, i64, i8*, i8* }* @__block_descriptor_tmp to %struct.__block_descriptor*) }, align 8

define void @block_fn() nounwind {
entry:
  ret void
}

define void @enqueue_simple_block(i32 addrspace(1)* %res) nounwind {
entry:
  %res.addr = alloca i32 addrspace(1)*, align 8
  %kernelBlock = alloca void ()*, align 8
  %def_q = alloca %opencl.queue_t*, align 8
  %ndrange = alloca %opencl.ndrange_t*, align 8
  %enq_res = alloca i32, align 4
  store i32 addrspace(1)* %res, i32 addrspace(1)** %res.addr, align 8
  store void ()* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor* }* @__block_literal_global to void ()*), void ()** %kernelBlock, align 8

; CHECK: %call = call %opencl.queue_t* @_Z17get_default_queuev()
  %call = call %opencl.queue_t* @_Z17get_default_queuev()
  
  store %opencl.queue_t* %call, %opencl.queue_t** %def_q, align 8
  %call1 = call %opencl.ndrange_t* @_Z10ndrange_1Dm(i64 1) nounwind readnone
  store %opencl.ndrange_t* %call1, %opencl.ndrange_t** %ndrange, align 8
  %0 = load %opencl.queue_t** %def_q, align 8
  %1 = load %opencl.ndrange_t** %ndrange, align 8
  %2 = load void ()** %kernelBlock, align 8

; CHECK: %call2 = call i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangeU13block_pointerFvvE(%opencl.queue_t* %0, i32 2, %opencl.ndrange_t* %1, void ()* %2)
  %call2 = call i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangeU13block_pointerFvvE(%opencl.queue_t* %0, i32 2, %opencl.ndrange_t* %1, void ()* %2) nounwind readnone
  store i32 %call2, i32* %enq_res, align 4
  ret void
}

define internal void @__enqueue_simple_block_block_invoke(i8* %.block_descriptor) nounwind {
entry:
  %block = bitcast i8* %.block_descriptor to <{ i8*, i32, i32, i8*, %struct.__block_descriptor* }>*
  call void @block_fn()
  ret void
}

declare %opencl.queue_t* @_Z17get_default_queuev()

declare %opencl.ndrange_t* @_Z10ndrange_1Dm(i64) nounwind readnone

declare i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangeU13block_pointerFvvE(%opencl.queue_t*, i32, %opencl.ndrange_t*, void ()*) nounwind readnone

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{void (i32 addrspace(1)*)* @enqueue_simple_block, !1}
!1 = !{!"argument_attribute", i32 0}
!2 = !{!"-cl-std=CL2.0"}
