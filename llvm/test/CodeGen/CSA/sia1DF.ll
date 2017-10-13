; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @sia1DF(double* %p, double** %p.addr) #0 {
; CSA_CHECK-LABEL: sia1DF
; CSA_CHECK: st64
; CSA_CHECK: ld64
; CSA_CHECK: st64

entry:
  store double* %p, double** %p.addr, align 8
  %0 = load double*, double** %p.addr, align 8
  %1 = load double, double* %0, align 8
  %add = fadd double %1, 1.000000e+00
  store double %add, double* %0, align 8
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
