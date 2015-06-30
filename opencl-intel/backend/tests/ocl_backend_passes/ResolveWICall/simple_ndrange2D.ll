; test ndrange_2D() is inlined
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
%opencl.ndrange_t = type opaque
declare %opencl.ndrange_t* @_Z10ndrange_2DPm(i64*) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_2DPmPm(i64*, i64*) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_2DPmPmPm(i64*, i64*, i64*) nounwind readnone

define void @enqueue_simple_block(i32 addrspace(1)* %res) nounwind {
  %gws = alloca [2 x i64], align 16
  %lws = alloca [2 x i64], align 16
  %gwo = alloca [2 x i64], align 16
  %gws_arr = bitcast [2 x i64]* %gws to i64*
  %lws_arr = bitcast [2 x i64]* %lws to i64*
  %gwo_arr = bitcast [2 x i64]* %gwo to i64*

  ; Calls should remain unchanged since they are in the built-in library
  ; CHECK: %call1 = call %opencl.ndrange_t* @_Z10ndrange_2DPm(i64* %gws_arr)
  ; CHECK: %call2 = call %opencl.ndrange_t* @_Z10ndrange_2DPmPm(i64* %gws_arr, i64* %lws_arr)
  ; CHECK: %call3 = call %opencl.ndrange_t* @_Z10ndrange_2DPmPmPm(i64* %gwo_arr, i64* %gws_arr, i64* %lws_arr)
  %call1 = call %opencl.ndrange_t* @_Z10ndrange_2DPm(i64* %gws_arr) nounwind readnone
  %call2 = call %opencl.ndrange_t* @_Z10ndrange_2DPmPm(i64* %gws_arr, i64* %lws_arr) nounwind readnone
  %call3 = call %opencl.ndrange_t* @_Z10ndrange_2DPmPmPm(i64* %gwo_arr, i64* %gws_arr, i64* %lws_arr) nounwind readnone
  ret void
}

!opencl.compiler.options = !{!2}
!2 = !{!"-cl-std=CL2.0"}
