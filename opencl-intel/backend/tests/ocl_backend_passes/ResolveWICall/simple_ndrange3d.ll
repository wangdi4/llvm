; test ndrange_3D() is inlined
; RUN: opt -add-implicit-args -resolve-wi-call -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
%opencl.ndrange_t = type opaque
declare %opencl.ndrange_t* @_Z10ndrange_3DPm(i64*) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_3DPmPm(i64*, i64*) nounwind readnone
declare %opencl.ndrange_t* @_Z10ndrange_3DPmPmPm(i64*, i64*, i64*) nounwind readnone

define void @enqueue_simple_block(i32 addrspace(1)* %res) nounwind {
; CHECK: alloca %struct.__ndrange_t
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t*
; CHECK-NEXT: store i32 3
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: bitcast %struct.__ndrange_t* %s.__ndrange_t to %opencl.ndrange_t*
; CHECK-NOT: call %opencl.ndrange_t* @_Z10ndrange_2DPm

; CHECK: alloca %struct.__ndrange_t
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t*
; CHECK-NEXT: store i32 3
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: bitcast %struct.__ndrange_t* %s.__ndrange_t1 to %opencl.ndrange_t*
; CHECK-NOT: call %opencl.ndrange_t* @_Z10ndrange_2DPmPm

; CHECK: alloca %struct.__ndrange_t
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t*
; CHECK-NEXT: store i32 3
; CHECK-NEXT: getelementptr inbounds i64* %gwo_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gwo_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gwo_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %gws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: getelementptr inbounds i64* %lws_arr
; CHECK-NEXT: load
; CHECK-NEXT: getelementptr inbounds %struct.__ndrange_t* 
; CHECK-NEXT: store i64
; CHECK-NEXT: bitcast %struct.__ndrange_t* %s.__ndrange_t3 to %opencl.ndrange_t*
; CHECK-NOT: call %opencl.ndrange_t* @_Z10ndrange_2DPmPmPm
  %gws = alloca [3 x i64], align 16
  %lws = alloca [3 x i64], align 16
  %gwo = alloca [3 x i64], align 16
  %gws_arr = bitcast [3 x i64]* %gws to i64*
  %lws_arr = bitcast [3 x i64]* %lws to i64*
  %gwo_arr = bitcast [3 x i64]* %gwo to i64*
  %call1 = call %opencl.ndrange_t* @_Z10ndrange_3DPm(i64* %gws_arr) nounwind readnone
  %call2 = call %opencl.ndrange_t* @_Z10ndrange_3DPmPm(i64* %gws_arr, i64* %lws_arr) nounwind readnone
  %call3 = call %opencl.ndrange_t* @_Z10ndrange_3DPmPmPm(i64* %gwo_arr, i64* %gws_arr, i64* %lws_arr) nounwind readnone
  ret void
}

!opencl.compiler.options = !{!2}
!2 = metadata !{metadata !"-cl-std=CL2.0"}
