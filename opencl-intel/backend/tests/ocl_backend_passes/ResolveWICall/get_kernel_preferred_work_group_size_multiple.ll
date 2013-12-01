; test get_kernel_preferred_work_group_size_multiple with(without) local memory arguments() are replaced with callbacks
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

define void @get_kernel_wg_size(void (i8 addrspace(3)*, ...)* %b, void ()* %a) nounwind {
; CHECK: call i32 @ocl20_get_kernel_preferred_wg_size_multiple_local
  %call1 = call i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvPU3AS3vzEjz(void (i8 addrspace(3)*, ...)* %b)
; CHECK: call i32 @ocl20_get_kernel_preferred_wg_size_multiple
  %call2 = call i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvvE(void ()* %a)
  
  ret void
}

; CHECK: declare i32 @ocl20_get_kernel_preferred_wg_size_multiple_local
declare i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvPU3AS3vzEjz(void (i8 addrspace(3)*, ...)*)
; CHECK: declare i32 @ocl20_get_kernel_preferred_wg_size_multiple
declare i32 @_Z45get_kernel_preferred_work_group_size_multipleU13block_pointerFvvE(void ()*)

!opencl.compiler.options = !{!2}
!2 = metadata !{metadata !"-cl-std=CL2.0"}