; test enqueue_marker are replaced with callback
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%opencl.clk_event_t = type opaque
%opencl.queue_t = type opaque

define void @test_enqueue_marker() nounwind {
entry:
  %marker_event = alloca %opencl.clk_event_t*, align 8
  %events = alloca [3 x %opencl.clk_event_t*], align 16
  %pqueue = alloca %opencl.queue_t* , align 16
  %queue = load %opencl.queue_t** %pqueue
  %arraydecay = getelementptr inbounds [3 x %opencl.clk_event_t*]* %events, i64 0, i64 0
; Call should remain unchanged since it is in the built-in library
; CHECK: %call1 = call i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(%opencl.queue_t* %queue, i32 2, %opencl.clk_event_t** %arraydecay, %opencl.clk_event_t** %marker_event)
  %call1 = call i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(%opencl.queue_t* %queue, i32 2, %opencl.clk_event_t** %arraydecay, %opencl.clk_event_t** %marker_event) nounwind
  ret void
}

declare i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(%opencl.queue_t*, i32, %opencl.clk_event_t**, %opencl.clk_event_t**)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
