; NEGATIVE test. In OpenCL1.2 mode enqueue_kernel IS NOT replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.queue_t = type opaque
%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.clk_event_t = type opaque

; CHECK: declare i32 @__enqueue_kernel_basic_events
declare i32 @__enqueue_kernel_basic_events(%opencl.queue_t*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t**, %opencl.clk_event_t**, i8 addrspace(4)*)

define void @enqueue_kernel_events(%opencl.queue_t* %q, %struct.ndrange_t* %nd, i8 addrspace(4)* %b, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret) nounwind {
; CHECK: call i32 @__enqueue_kernel_basic_events
%call5 = call i32 @__enqueue_kernel_basic_events(%opencl.queue_t* %q, i32 0, %struct.ndrange_t* %nd, i32 1, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret, i8 addrspace(4)* %b)
  ret void
}

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL1.2"}
