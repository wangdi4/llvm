; test retain_event and release_event are replaced with callback
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.clk_event_t = type opaque

define void @test_retain_event() nounwind {
entry:
  %ptr_event = alloca %opencl.clk_event_t*, align 8
  %event = load %opencl.clk_event_t** %ptr_event
; CHECK: call void @_Z12retain_event12ocl_clkevent(%opencl.clk_event_t* %event)
; CHECK: call void @_Z13release_event12ocl_clkevent(%opencl.clk_event_t* %event)
  call void @_Z12retain_event12ocl_clkevent(%opencl.clk_event_t* %event) nounwind
  call void @_Z13release_event12ocl_clkevent(%opencl.clk_event_t* %event) nounwind
  ret void
}

declare void @_Z12retain_event12ocl_clkevent(%opencl.clk_event_t*)
declare void @_Z13release_event12ocl_clkevent(%opencl.clk_event_t*)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
