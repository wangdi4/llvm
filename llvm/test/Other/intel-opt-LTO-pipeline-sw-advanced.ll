; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; RUN: opt -enable-new-pm=0 -mtriple=x86_64-- -std-link-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck --check-prefix=CHECK %s

; CHECK:     Intel IPO Prefetch
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Post-Dominator Tree Construction
; CHECK: Pass Arguments:  -domtree -postdomtree
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT:     Post-Dominator Tree Construction

define void @f() {
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
