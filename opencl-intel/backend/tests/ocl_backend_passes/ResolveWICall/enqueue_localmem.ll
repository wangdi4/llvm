; test enqueue_kernel with local memory arguments() are replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.clk_event_t = type opaque


define void @enqueue_kernel_localmem(%opencl.queue_t addrspace(1)* %q, %struct.ndrange_t* %nd, void (i8 addrspace(3)*, ...)* %b, i32 %localMem1, i32 %localMem2) nounwind {
; CHECK: call i32 @ocl20_enqueue_kernel_localmem
  %call = call i32 (%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t*, void (i8 addrspace(3)*, ...)*, i32, ...)* @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvPU3AS3vzEjz(%opencl.queue_t addrspace(1)* %q, i32 0, %struct.ndrange_t* %nd, void (i8 addrspace(3)*, ...)* %b, i32 %localMem1, i32 %localMem2)
  ret void
}

; CHECK: declare i32 @ocl20_enqueue_kernel_localmem
declare i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvPU3AS3vzEjz(%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t* byval, void (i8 addrspace(3)*, ...)*, i32, ...)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
