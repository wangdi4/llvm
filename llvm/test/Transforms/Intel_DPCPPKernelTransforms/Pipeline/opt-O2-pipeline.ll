; RUN: opt -O2 -enable-dpcpp-kernel-transforms -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK-LABEL: Pass Arguments:
; CHECK-LABEL: Pass Arguments:
; CHECK:         ModulePass Manager
; CHECK:           Parse annotations and add a corresponding function attribute
; CHECK-NEXT:      DPCPPEqualizerLegacy
; CHECK-NEXT:      DPCPPKernelAnalysisLegacy
; CHECK-NEXT:      DPCPPKernelVecClone pass
; CHECK-NEXT:      VecClone
; CHECK-NEXT:      FunctionPass Manager
; CHECK:             VPlan Vectorization Driver
; CHECK:           VPlan post vectorization pass for DPCPP kernels
; CHECK-NEXT:      FunctionPass Manager
; CHECK:             VPO Directive Cleanup
; CHECK:             Combine redundant instructions
; CHECK-NEXT:        Simplify the CFG
; CHECK-NEXT:        Dominator Tree Construction
; CHECK-NEXT:        Promote Memory to Register
; CHECK-NEXT:        Post-Dominator Tree Construction
; CHECK-NEXT:        Aggressive Dead Code Elimination
; CHECK-NEXT:        Unify function exit nodes
; CHECK-NEXT:      WGLoopCreatorLegacy
; CHECK-NEXT:        FunctionPass Manager
; CHECK-NEXT:          Unify function exit nodes
; CHECK:           ImplicitArgsAnalysisLegacy
; CHECK:           LocalBufferAnalysisLegacy
; CHECK-NEXT:      AddImplicitArgsLegacy
; CHECK-NEXT:      ResolveWICallLegacy
; CHECK-NEXT:      PrepareKernelArgsLegacy
; CHECK:           CleanupWrappedKernelLegacy

define void @f() {
  ret void
}
