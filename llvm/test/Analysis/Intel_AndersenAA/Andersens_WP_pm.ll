; Testing that WholeProgramAnalysis is not invalidated until
; AndersensAA is called second time (just before LoopOpt) during LTO
; compilation pipeline.
; New PM: opt with 'lto<O3>' option is used to simulate LTO pass pipeline.
; REQUIRES: asserts

; RUN: opt %s -o Andersens_WP_pm.bc
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
