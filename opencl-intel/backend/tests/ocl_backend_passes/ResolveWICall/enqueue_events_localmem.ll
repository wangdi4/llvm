; Not implemented in branch
; test enqueue_kernel with events are replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.queue_t = type opaque
%opencl.ndrange_t = type opaque
%opencl.clk_event_t = type opaque


define void @enqueue_kernel_events(%opencl.queue_t* %q, %opencl.ndrange_t* %nd, void (i8 addrspace(3)*, ...)* %b, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret ) nounwind {
; CHECK: call {{.*}} @ocl20_enqueue_kernel_events_localmem
; CHECK-NOT: call {{.*}} _Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS1K12ocl_clkevent_S1U13block_pointerFvPU3AS3vzEjz
%call5 = call i32 (%opencl.queue_t*, i32, %opencl.ndrange_t*, i32, %opencl.clk_event_t**, %opencl.clk_event_t**, void (i8 addrspace(3)*, ...)*, i32, ...) @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS1K12ocl_clkeventS1_U13block_pointerFvPU3AS3vzEjz(%opencl.queue_t* %q, i32 0, %opencl.ndrange_t* %nd, i32 1, %opencl.clk_event_t** %evt0, %opencl.clk_event_t** %evt_ret, void (i8 addrspace(3)*, ...)* %b, i32 64, i32 1024, i32 128)
    ret void
}

declare i32 @_Z14enqueue_kernel9ocl_queuei9ndrange_tjPU3AS1K12ocl_clkeventS1_U13block_pointerFvPU3AS3vzEjz(%opencl.queue_t*, i32, %opencl.ndrange_t*, i32, %opencl.clk_event_t**, %opencl.clk_event_t**, void (i8 addrspace(3)*, ...)*, i32, ...)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
