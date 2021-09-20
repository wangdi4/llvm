; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; RUN: opt -mtriple=x86_64-- -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck --check-prefix=CHECK %s

; CHECK:     Intel IPO Prefetch
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Post-Dominator Tree Construction

; CHECK:     DeadArrayOpsElimination
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Scalar Evolution Analysis
; CHECK-NEXT:         Array Use Analysis

; CHECK:            Unaligned Nontemporal Store Conversion

; CHECK: Pass Arguments:  -domtree -postdomtree
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT:     Post-Dominator Tree Construction


define void @f() {
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
