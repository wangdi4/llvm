; RUN: opt -generic-addr-dynamic-resolution -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;; This test was generated using the following cl code with this command:
;;  clang -cc1 -cl-std=CL2.0 -x cl -emit-llvm -include opencl_.h -I <Path-TO>\clang_headers\ -g -O0 -D__OPENCL_C_VERSION__=200 -o result.ll
;;
;;__kernel void ker( __global int* zuzu)
;;{
;;    event_t ev;
;;    wait_group_events(1, &ev);
;;}

; The test checks that function "wait_group_events" resolve with GAS argument.
; CHECK: [[VAR:%.*]] = addrspacecast %opencl.event_t* addrspace(4)* %{{.*}} to %opencl.event_t**
; CHECK: call void @_Z17wait_group_eventsiP9ocl_event(i32 1, %opencl.event_t** [[VAR]])
; CHECK: declare void @_Z17wait_group_eventsiP9ocl_event(i32, %opencl.event_t**)

; ModuleID = '/tmp/s.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%opencl.event_t = type opaque

; Function Attrs: nounwind
define void @ker(i32 addrspace(1)* %zuzu) #0 {
  %1 = alloca i32 addrspace(1)*, align 8
  %ev = alloca %opencl.event_t*, align 8
  store i32 addrspace(1)* %zuzu, i32 addrspace(1)** %1, align 8
  %2 = addrspacecast %opencl.event_t** %ev to %opencl.event_t* addrspace(4)*
  call void @_Z17wait_group_eventsiPU3AS49ocl_event(i32 1, %opencl.event_t* addrspace(4)* %2)
  ret void
}

declare void @_Z17wait_group_eventsiPU3AS49ocl_event(i32, %opencl.event_t* addrspace(4)*) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!1}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*)* @ker}
!1 = !{!"-cl-std=CL2.0"}
