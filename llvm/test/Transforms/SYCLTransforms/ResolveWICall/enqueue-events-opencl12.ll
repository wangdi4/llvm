; NEGATIVE test. In OpenCL1.2 mode enqueue_kernel IS NOT replaced with callbacks
; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

%struct.ndrange_t = type { i32, [3 x i64], [3 x i64], [3 x i64] }

; CHECK: declare i32 @__enqueue_kernel_basic_events
declare i32 @__enqueue_kernel_basic_events(ptr, i32, ptr, i32, ptr, ptr, ptr addrspace(4))

define void @enqueue_kernel_events(ptr %q, ptr %nd, ptr addrspace(4) %b, ptr %evt0, ptr %evt_ret) nounwind !kernel_arg_base_type !1 !arg_type_null_val !2 {
; CHECK: call i32 @__enqueue_kernel_basic_events
%call5 = call i32 @__enqueue_kernel_basic_events(ptr %q, i32 0, ptr %nd, i32 1, ptr %evt0, ptr %evt_ret, ptr addrspace(4) %b)
  ret void
}

!opencl.compiler.options = !{!0}
!0 = !{!"-cl-std=CL1.2"}
!1 = !{!"queue_t", !"%struct.ndrange_t.2*", !"char*", !"clk_event_t*", !"clk_event_t*"}
!2 = !{target("spirv.Queue") zeroinitializer, ptr null, ptr addrspace(4) null, ptr null, ptr null}

; DEBUGIFY-NOT: WARNING
