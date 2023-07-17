; test create_user_event and set_user_event_status are replaced with callback
; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1  | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

define void @test_create_event_and_set_event_status() nounwind {
entry:
  ; Calls should remain unchanged since they are in the built-in library
  ; CHECK: %event = call ptr @_Z17create_user_eventv()
  ; CHECK: call void @_Z21set_user_event_status12ocl_clkeventi(ptr %event, i32 0)
  %event = call ptr @_Z17create_user_eventv() nounwind
  call void @_Z21set_user_event_status12ocl_clkeventi(ptr %event, i32 0) nounwind
  ret void
}

declare ptr @_Z17create_user_eventv()
declare void @_Z21set_user_event_status12ocl_clkeventi(ptr, i32)

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}

; DEBUGIFY-NOT: WARNING
