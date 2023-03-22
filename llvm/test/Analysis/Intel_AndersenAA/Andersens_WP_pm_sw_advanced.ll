; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Testing that WholeProgramAnalysis is not invalidated until
; AndersensAA is called second time (just before LoopOpt) during LTO
; compilation pipeline.
; New PM: opt with 'lto<O3>' option is used to simulate LTO pass pipeline.

; This is the same test as Andersens_WP_pm.ll, but it tests the passes
; available when INTEL FEATURE SW_ADVANCED is enabled.

; RUN: opt %s -o Andersens_WP_pm_sw_advanced.bc
; RUN: opt -passes='lto<O3>' -debug-pass-manager 2>&1 | FileCheck %s

; CHECK: Running analysis: WholeProgramAnalysis on [module]
;
; CHECK-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK: Running analysis: AndersensAA on [module]
;
; CHECK-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK: Invalidating analysis: AndersensAA on [module]
;
; CHECK: Running analysis: AndersensAA on [module]

define void @main()  {
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
