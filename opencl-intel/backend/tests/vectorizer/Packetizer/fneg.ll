; Tests the LLVM fneg instruction is vectorized properly

; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -packetize -packet-size=16 -verify %s -S -o - \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i64 @_Z13get_global_idj(i64) nounwind readnone

define void @fneg_float(float addrspace(1)* nocapture %X, float addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK-LABEL: @fneg_float
; CHECK: [[LD:%.*]] = load <16 x float>, <16 x float> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 4
; CHECK: fneg <16 x float> [[LD]]
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds float, float addrspace(1)* %X, i64 %1
  %3 = load float, float addrspace(1)* %2
  %4 = fneg float %3
  %5 = getelementptr inbounds float, float addrspace(1)* %Y, i64 %1
  store float %4, float addrspace(1)* %5
  ret void
}


define void @fneg_double(double addrspace(1)* nocapture %X, double addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK-LABEL: @fneg_double
; CHECK: [[LD:%.*]] = load <16 x double>, <16 x double> addrspace(1)* %{{[a-zA-Z0-9_.]+}}, align 8
; CHECK: fneg <16 x double> [[LD]]
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds double, double addrspace(1)* %X, i64 %1
  %3 = load double, double addrspace(1)* %2
  %4 = fneg double %3
  %5 = getelementptr inbounds double, double addrspace(1)* %Y, i64 %1
  store double %4, double addrspace(1)* %5
  ret void
}
