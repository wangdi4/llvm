; test enqueue_marker are replaced with callback
; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

define void @test_enqueue_marker() nounwind {
entry:
  %marker_event = alloca ptr, align 8
  %events = alloca [3 x ptr], align 16
  %pqueue = alloca ptr , align 16
  %queue = load ptr, ptr %pqueue
; Call should remain unchanged since it is in the built-in library
; CHECK: %call1 = call i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(ptr %queue, i32 2, ptr %events, ptr %marker_event)
  %call1 = call i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(ptr %queue, i32 2, ptr %events, ptr %marker_event) nounwind
  ret void
}

declare i32 @_Z14enqueue_marker9ocl_queuejPK12ocl_clkeventP12ocl_clkevent(ptr, i32, ptr, ptr)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}

; DEBUGIFY-NOT: WARNING
