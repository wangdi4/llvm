; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define float @rinFF(float* %p) #0 {
; LPU_CHECK-LABEL: rinFF
; LPU_CHECK: negf32
; LPU_CHECK-NOT: sext32

entry:
  %p.addr = alloca float*, align 8
  store float* %p, float** %p.addr, align 8
  %0 = load float** %p.addr, align 8
  %1 = load float* %0, align 4
  %sub = fsub float -0.000000e+00, %1
  ret float %sub
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}