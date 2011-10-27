; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtime=ocl -runtimelib %p/../Full/runtime.bc -packetize -packet-size=4 -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare float @_Z5acoshf(float)

declare i64 @get_global_id(i32)

;  make sure that the WIAnalysis detects the ptr as uniform
; CHECK: __Vectorized_wlacosh
; CHECK: store
; CHECK-NOT: store
; CHECK: ret

define void @__Vectorized_wlacosh(float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @get_global_id(i32 0) nounwind
  %2 = and i64 %1, 4294967295
  %3 = getelementptr inbounds float addrspace(1)* %input, i64 %2
  %4 = load float addrspace(1)* %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float addrspace(1)* %output, i64 %2
  store float %5, float addrspace(1)* %6, align 4
  ret void
}

;  make sure that the WIAnalysis does NOT detect the ptr as uniform
; CHECK: __Vectorized_wlacosh2
; CHECK: store
; CHECK: store
; CHECK: ret

define void @__Vectorized_wlacosh2(float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @get_global_id(i32 0) nounwind
  %2 = and i64 %1, 4294967000  ; <--------notice the constant change
  %3 = getelementptr inbounds float addrspace(1)* %input, i64 %2
  %4 = load float addrspace(1)* %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float addrspace(1)* %output, i64 %2
  store float %5, float addrspace(1)* %6, align 4
  ret void
}

;  make sure that the WIAnalysis does NOT detect the ptr as uniform
; CHECK: __Vectorized_wlacosh3
; CHECK: store
; CHECK: store
; CHECK: ret

define void @__Vectorized_wlacosh3(float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @get_global_id(i32 0) nounwind
  %2 = and i64 %1, 123415  ; <--------notice the constant change
  %3 = getelementptr inbounds float addrspace(1)* %input, i64 %2
  %4 = load float addrspace(1)* %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float addrspace(1)* %output, i64 %2
  store float %5, float addrspace(1)* %6, align 4
  ret void
}
