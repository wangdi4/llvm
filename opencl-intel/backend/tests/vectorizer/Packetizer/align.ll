; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=16 -verify %s -S -o - \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i64 @_Z13get_global_idj(i64) nounwind readnone

define void @explicit1(float addrspace(1)* nocapture %X, float addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK: load <16 x float>, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 4
; CHECK: store <16 x float> %{{[a-zA-Z0-9_.]+}}, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 4
; CHECK: ret
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds float, float addrspace(1)* %X, i64 %1
  %3 = load float, float addrspace(1)* %2
  %4 = fadd float %3, 1.000000e+00
  %5 = getelementptr inbounds float, float addrspace(1)* %Y, i64 %1
  store float %4, float addrspace(1)* %5
  ret void
}

define void @explicit2(i1 addrspace(1)* nocapture %X, i1 addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK: load <16 x i1>, <16 x i1> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 1
; CHECK: store <16 x i1> %{{[a-zA-Z0-9_.]+}}, <16 x i1> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 1
; CHECK: ret
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds i1, i1 addrspace(1)* %X, i64 %1
  %3 = load i1, i1 addrspace(1)* %2
  %4 = add i1 %3, 1
  %5 = getelementptr inbounds i1, i1 addrspace(1)* %Y, i64 %1
  store i1 %4, i1 addrspace(1)* %5
  ret void
}

define void @preserve1(float addrspace(1)* nocapture %X, float addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK: load <16 x float>, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 4
; CHECK: store <16 x float> %{{[a-zA-Z0-9_.]+}}, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 4
; CHECK: ret
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds float, float addrspace(1)* %X, i64 %1
  %3 = load float, float addrspace(1)* %2, align 4
  %4 = fadd float %3, 1.000000e+00
  %5 = getelementptr inbounds float, float addrspace(1)* %Y, i64 %1
  store float %4, float addrspace(1)* %5, align 4
  ret void
}

define void @preserve2(float addrspace(1)* nocapture %X, float addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK: load <16 x float>, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 64
; CHECK: store <16 x float> %{{[a-zA-Z0-9_.]+}}, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 64
; CHECK: ret
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds float, float addrspace(1)* %X, i64 %1
  %3 = load float, float addrspace(1)* %2, align 64
  %4 = fadd float %3, 1.000000e+00
  %5 = getelementptr inbounds float, float addrspace(1)* %Y, i64 %1
  store float %4, float addrspace(1)* %5, align 64
  ret void
}
