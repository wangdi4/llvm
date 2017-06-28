; Check pass correctly handles number appended to type name
; Happens when the same bitcode loading happens to the same Context twice
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.queue_t.7 = type opaque
%opencl.ndrange_t.2 = type opaque
%opencl.clk_event_t.4 = type opaque


define void @enqueue_kernel_events(%opencl.queue_t.7* %q, %opencl.ndrange_t.2* %nd, void ()* %b, %opencl.clk_event_t.4** %evt0, %opencl.clk_event_t.4** %evt_ret) nounwind {
; Call should remain unchanged since it is in the built-in library
; CHECK: call i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPU3AS4S1_U13block_pointerFvvE(%opencl.queue_t.7* %q, i32 0, %opencl.ndrange_t.2* %nd, i32 1, %opencl.clk_event_t.4** %evt0, %opencl.clk_event_t.4** %evt_ret, void ()* %b)
%call5 = call i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPU3AS4S1_U13block_pointerFvvE(%opencl.queue_t.7* %q, i32 0, %opencl.ndrange_t.2* %nd, i32 1, %opencl.clk_event_t.4** %evt0, %opencl.clk_event_t.4** %evt_ret, void ()* %b)
  ret void
}

declare i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS4K12ocl_clkeventPU3AS4S1_U13block_pointerFvvE(%opencl.queue_t.7*, i32, %opencl.ndrange_t.2*, i32, %opencl.clk_event_t.4**, %opencl.clk_event_t.4**, void ()*)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
