; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts, intel_feature_sw_advanced

; Testing that WholeProgramAnalysis is not invalidated until
; AndersensAA is called second time (just before LoopOpt) during LTO
; compilation pipeline.
; Old PM: llvm-lto tool is used instead of opt to simulate LTO pass pipeline.
; New PM: opt with 'lto<O3>' option is used to simulate LTO pass pipeline.

; This is the same test as Andersens_WP_pm.ll, but it tests the passes
; available when INTEL FEATURE SW_ADVANCED is enabled.

; RUN: opt %s -o Andersens_WP_pm_sw_advanced.bc
; RUN: llvm-lto -use-new-pm=false -disable-verify -debug-pass=Executions -O3 Andersens_WP_pm_sw_advanced.bc 2>&1 | FileCheck %s
; RUN: opt -passes='lto<O3>' -debug-pass-manager 2>&1 | FileCheck %s --check-prefix=CHECK-NEW-PM

; CHECK: Executing Pass 'Whole program analysis' on Module
;
; CHECK-NOT: Freeing Pass 'Whole program analysis' on Module
;
; CHECK: Freeing Pass 'Intel IPO Prefetch' on Module
;
; CHECK: Executing Pass 'Andersen Interprocedural AA' on Module
;
; CHECK-NOT: Freeing Pass 'Whole program analysis' on Module
;
; CHECK: Executing Pass 'Andersen Interprocedural AA' on Module
;
; CHECK: Freeing Pass 'Whole program analysis' on Module

; CHECK-NEW-PM: Running analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM: Running analysis: AndersensAA on [module]
;
; CHECK-NEW-PM-NOT: Invalidating analysis: WholeProgramAnalysis on [module]
;
; CHECK-NEW-PM: Invalidating analysis: AndersensAA on [module]
;
; CHECK-NEW-PM: Running analysis: AndersensAA on [module]

define void @main()  {
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
