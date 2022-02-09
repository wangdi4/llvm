; XFAIL: *

; test enqueue_kernel with local memory arguments() are replaced with callbacks
; RUN: opt -dpcpp-kernel-add-implicit-args -debugify -dpcpp-kernel-resolve-wi-call -check-debugify -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-resolve-wi-call -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }
%opencl.queue_t = type opaque
%opencl.clk_event_t = type opaque

@__block_literal_global = internal addrspace(1) constant { i32, i32 } { i32 8, i32 4 }, align 4

define void @enqueue_kernel_localmem(%opencl.queue_t* %q, %struct.ndrange_t* %nd, %opencl.clk_event_t* addrspace(4)* %event1, %opencl.clk_event_t* addrspace(4)* %event2) nounwind {

; CHECK:     call i32 @__ocl20_enqueue_kernel_localmem
; CHECK-NOT: call i32 @__enqueue_kernel_vaargs

  %localsize = alloca [1 x i64]
  %gep = getelementptr [1 x i64], [1 x i64]* %localsize, i32 0, i32 0
  store i64 256, i64* %gep, align 8
  %call1 = call i32 @__enqueue_kernel_vaargs(%opencl.queue_t* %q, i32 0, %struct.ndrange_t* %nd, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*)* @__device_side_enqueue_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32 } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), i32 1, i64* %gep)

; CHECK:     call i32 @__ocl20_enqueue_kernel_events_localmem
; CHECK-NOT: call i32 @__enqueue_kernel_events_vaargs

  %call2 = call i32 @__enqueue_kernel_events_vaargs(%opencl.queue_t* %q, i32 0, %struct.ndrange_t* %nd, i32 2, %opencl.clk_event_t* addrspace(4)* %event1, %opencl.clk_event_t* addrspace(4)* %event2, i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8 addrspace(4)*, i8 addrspace(3)*)* @__device_side_enqueue_block_invoke_kernel to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32 } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), i32 1, i64* %gep)
  ret void
}

; CHECK: declare i32 @__ocl20_enqueue_kernel_localmem
; CHECK: declare i32 @__ocl20_enqueue_kernel_events_localmem

declare i32 @__enqueue_kernel_events_vaargs(%opencl.queue_t*, i32, %struct.ndrange_t*, i32, %opencl.clk_event_t* addrspace(4)*, %opencl.clk_event_t* addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i64*)

declare i32 @__enqueue_kernel_vaargs(%opencl.queue_t*, i32, %struct.ndrange_t*, i8 addrspace(4)*, i8 addrspace(4)*, i32, i64*)

define void @__device_side_enqueue_block_invoke_kernel(i8 addrspace(4)*, i8 addrspace(3)*) {
entry:
  ret void
}

!opencl.ocl.version = !{!0}
!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
