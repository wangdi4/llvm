; test create_user_event and set_user_event_status are replaced with callback
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.clk_event_t = type opaque

define void @test_create_event_and_set_event_status() nounwind {
entry:
  ; CHECK: call %opencl.clk_event_t* @ocl20_create_user_event
  %event = call %opencl.clk_event_t* @_Z17create_user_eventv() nounwind
  ; CHECK: call void @ocl20_set_user_event_status
  call void @_Z21set_user_event_status13ocl_clk_eventi(%opencl.clk_event_t* %event, i32 0) nounwind
  ret void
}

; CHECK: declare %opencl.clk_event_t* @ocl20_create_user_event
declare %opencl.clk_event_t* @_Z17create_user_eventv()
; CHECK: declare void @ocl20_set_user_event_status
declare void @_Z21set_user_event_status13ocl_clk_eventi(%opencl.clk_event_t*, i32)

!opencl.compiler.options = !{!2}
!2 = metadata !{metadata !"-cl-std=CL2.0"}