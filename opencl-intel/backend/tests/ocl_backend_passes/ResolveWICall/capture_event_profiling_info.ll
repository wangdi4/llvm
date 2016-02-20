; test capture_event_profiling_info are replaced with callback
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.clk_event_t = type opaque

define void @test_capture_event_profiling_info(i64 addrspace(1)* %value) nounwind {
entry:
  %p_event = alloca %opencl.clk_event_t*, align 8
  %event = load %opencl.clk_event_t** %p_event
  ; Call should remain unchanged since it is in the built-in library
  ; CHECK: call void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(%opencl.clk_event_t* %event, i32 1, i64 addrspace(1)* %value)
  call void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(%opencl.clk_event_t* %event, i32 1, i64 addrspace(1)* %value) nounwind
  ret void
}

declare void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(%opencl.clk_event_t*, i32, i64 addrspace(1)*)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
