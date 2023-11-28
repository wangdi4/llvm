; INTEL_CUSTOMIZATION
; Note: -paropt=11 is an equivalent of -fiopenmp-simd for the driver and we do
; vectorization at O0 only when -O0 and -fiopenmp (or -fiopenmp-simd) are set.
; RUN: opt -passes='default<O0>' -paropt=11 -debug-pass-manager -enable-o0-vectorization < %s 2>&1 | FileCheck %s

; CHECK: Running pass: vpo::VPlanDriverLLVMPass

define void @foo() {ret void}
; end INTEL_CUSTOMIZATION
