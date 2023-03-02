; INTEL_CUSTOMIZATION
; RUN: opt -passes='default<O0>' -debug-pass-manager -enable-o0-vectorization < %s 2>&1 | FileCheck %s

; CHECK: Running pass: vpo::VPlanDriverPass

define void @foo() {ret void}
; end INTEL_CUSTOMIZATION
