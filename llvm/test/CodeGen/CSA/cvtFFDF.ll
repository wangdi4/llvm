; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define float @cvtFFDF(double %d) #0 {
; CSA_CHECK-LABEL: cvtFFDF
; CSA_CHECK: cvtf32f64

entry:
  %d.addr = alloca double, align 8
  store double %d, double* %d.addr, align 8
  %0 = load double, double* %d.addr, align 8
  %conv = fptrunc double %0 to float
  ret float %conv
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}