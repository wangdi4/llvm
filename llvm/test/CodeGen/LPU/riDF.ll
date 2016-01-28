; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define double @riDF(double* %p) #0 {
; LPU_CHECK-LABEL: riDF
; LPU_CHECK-NOT: sext32

entry:
  %p.addr = alloca double*, align 8
  store double* %p, double** %p.addr, align 8
  %0 = load double** %p.addr, align 8
  %1 = load double* %0, align 8
  ret double %1
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}