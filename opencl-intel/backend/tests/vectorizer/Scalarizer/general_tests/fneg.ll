; Tests the LLVM fneg instruction is scalarized properly

; RUN: %oclopt  -runtimelib %p/../../Full/runtime.bc -scalarize -verify %s -S -o - \
; RUN: | FileCheck %s

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i64 @_Z13get_global_idj(i64) nounwind readnone

define void @fneg_float(<4 x float> addrspace(1)* nocapture %X, <4 x float> addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK-LABEL: @fneg_float
; CHECK: fneg float %{{.*}}
; CHECK-NEXT: fneg float %{{.*}}
; CHECK-NEXT: fneg float %{{.*}}
; CHECK-NEXT: fneg float %{{.*}}
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %X, i64 %1
  %3 = load <4 x float>, <4 x float> addrspace(1)* %2
  %4 = fneg <4 x float> %3
  %5 = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %Y, i64 %1
  store <4 x float> %4, <4 x float> addrspace(1)* %5
  ret void
}

define void @fneg_double(<4 x double> addrspace(1)* nocapture %X, <4 x double> addrspace(1)* nocapture %Y) nounwind uwtable {
; CHECK-LABEL: @fneg_double
; CHECK: fneg double %{{.*}}
; CHECK-NEXT: fneg double %{{.*}}
; CHECK-NEXT: fneg double %{{.*}}
; CHECK-NEXT: fneg double %{{.*}}
  %1 = tail call i64 @_Z13get_global_idj(i64 0) nounwind
  %2 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %X, i64 %1
  %3 = load <4 x double>, <4 x double> addrspace(1)* %2
  %4 = fneg <4 x double> %3
  %5 = getelementptr inbounds <4 x double>, <4 x double> addrspace(1)* %Y, i64 %1
  store <4 x double> %4, <4 x double> addrspace(1)* %5
  ret void
}
