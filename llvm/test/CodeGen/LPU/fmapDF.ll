; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define double @fmapDF(double %a, double %b, double %c, double %d) #0 {
; LPU_CHECK-LABEL: fmapDF
; LPU_CHECK: fmaf64
; LPU_CHECK: addf64

entry:
  %a.addr = alloca double, align 8
  %b.addr = alloca double, align 8
  %c.addr = alloca double, align 8
  %d.addr = alloca double, align 8
  store double %a, double* %a.addr, align 8
  store double %b, double* %b.addr, align 8
  store double %c, double* %c.addr, align 8
  store double %d, double* %d.addr, align 8
  %0 = load double* %a.addr, align 8
  %1 = load double* %b.addr, align 8
  %mul = fmul double %0, %1
  %2 = load double* %c.addr, align 8
  %add = fadd double %mul, %2
  %3 = load double* %d.addr, align 8
  %add1 = fadd double %add, %3
  ret double %add1
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}