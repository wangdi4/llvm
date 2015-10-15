; NEGATIVE test. In OpenCL1.2 mode enqueue_kernel IS NOT replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.queue_t = type opaque
%opencl.ndrange_t = type opaque
%opencl.clk_event_t = type opaque

; CHECK: declare i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangejPK13ocl_clk_eventP13ocl_clk_eventU13block_pointerFvvE
declare i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangejPK13ocl_clk_eventP13ocl_clk_eventU13block_pointerFvvE(%opencl.queue_t*, i32, %opencl.ndrange_t*, i32, %opencl.clk_event_t**, %opencl.clk_event_t**, void ()*)

define void @enqueue_kernel_events(%opencl.queue_t* %q, %opencl.ndrange_t* %nd, void ()* %b, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret) nounwind {
; CHECK: call i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangejPK13ocl_clk_eventP13ocl_clk_eventU13block_pointerFvvE
%call5 = call i32 @_Z14enqueue_kernel9ocl_queuei11ocl_ndrangejPK13ocl_clk_eventP13ocl_clk_eventU13block_pointerFvvE(%opencl.queue_t* %q, i32 0, %opencl.ndrange_t* %nd, i32 1, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret, void ()* %b)
  ret void
}


!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL1.2"}