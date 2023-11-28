; Test to verify that vector-variant function declarations are always added to
; @llvm.compiler.used, even in cases when vector function's declaration is
; already available in the module.

; REQUIRES: asserts
; RUN: opt -vector-library=SVML       -passes=inject-tli-mappings -S -debug-only=vectorutils < %s 2>&1 | FileCheck %s

; CHECK: VFABI: adding mapping '_ZGV_LLVM_N8v_llvm.log.f64(__svml_log8)'

; CHECK-LABEL: @llvm.compiler.used = appending global
; CHECK-SAME:        [12 x ptr] [
; CHECK-SAME:          ptr @__svml_log2,
; CHECK-SAME:          ptr @__svml_log4,
; CHECK-SAME:          ptr @__svml_log8,
; CHECK-SAME:          ptr @__svml_log16,
; CHECK-SAME:          ptr @__svml_log32,
; CHECK-SAME:          ptr @__svml_log64
; CHECK-SAME:      ], section "llvm.metadata"


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define double @log_f64(double %in) {
  %call = tail call double @llvm.log.f64(double %in) #1
  ret double %call
}

declare double @llvm.log.f64(double) #0

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare <8 x double> @__svml_log8(<8 x double>) #2

attributes #0 = { nounwind readnone }
attributes #1 = { "vector-function-abi-variant"="_ZGV_LLVM_N8v_llvm.log.f64(__svml_log8)" }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
