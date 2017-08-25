; test enqueue_kernel with local memory arguments() are replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.clk_event_t = type opaque

define void @enqueue_kernel_localmem(%opencl.queue_t addrspace(1)* %q, %struct.ndrange_t* %nd, %opencl.clk_event_t* addrspace(4)* %event1, %opencl.clk_event_t* addrspace(4)* %event2, i8 addrspace(4)* %b, i32 %localMem1, i32 %localMem2) nounwind {

; CHECK:     call i32 @ocl20_enqueue_kernel_localmem
; CHECK-NOT: call i32 @__enqueue_kernel_vaargs

  %call1 = call i32 (%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t*, i8 addrspace(4)*, i32, ...) @__enqueue_kernel_vaargs(%opencl.queue_t addrspace(1)* %q, i32 0, %struct.ndrange_t* %nd, i8 addrspace(4)* %b, i32 %localMem1, i32 %localMem2)

; CHECK:     call i32 @ocl20_enqueue_kernel_events_localmem
; CHECK-NOT: call i32 @__enqueue_kernel_events_vaargs

  %call2 = call i32 (%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t* addrspace(4)*, %opencl.clk_event_t* addrspace(4)*, i8 addrspace(4)*, i32, ...) @__enqueue_kernel_events_vaargs(%opencl.queue_t addrspace(1)* %q, i32 0, %struct.ndrange_t* %nd, i32 2, %opencl.clk_event_t* addrspace(4)* %event1, %opencl.clk_event_t* addrspace(4)* %event2, i8 addrspace(4)* %b, i32 %localMem1, i32 %localMem2)
  ret void
}

; CHECK: declare i32 @ocl20_enqueue_kernel_localmem
; CHECK: declare i32 @ocl20_enqueue_kernel_events_localmem

declare i32 @__enqueue_kernel_vaargs(%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t* byval, i8 addrspace(4)*, i32, ...)
declare i32 @__enqueue_kernel_events_vaargs(%opencl.queue_t addrspace(1)*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t* addrspace(4)*, %opencl.clk_event_t* addrspace(4)*, i8 addrspace(4)*, i32, ...)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
