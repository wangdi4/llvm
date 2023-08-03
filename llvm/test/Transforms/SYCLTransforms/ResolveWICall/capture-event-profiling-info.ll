; test capture_event_profiling_info are replaced with callback
; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

define void @test_capture_event_profiling_info(ptr addrspace(1) %value) nounwind !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %p_event = alloca ptr, align 8
  %event = load ptr, ptr %p_event
  ; Call should remain unchanged since it is in the built-in library
  ; CHECK: call void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(ptr %event, i32 1, ptr addrspace(1) %value)
  call void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(ptr %event, i32 1, ptr addrspace(1) %value) nounwind
  ret void
}

declare void @_Z28capture_event_profiling_info12ocl_clkeventiPU3AS1m(ptr, i32, ptr addrspace(1))

!opencl.compiler.options = !{!0}
!0 = !{!"-cl-std=CL2.0"}
!1 = !{!"long*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
