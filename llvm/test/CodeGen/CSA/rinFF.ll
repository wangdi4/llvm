; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define float @rinFF(float* %p) #0 {
; CSA_CHECK-LABEL: rinFF
; CSA_CHECK: negf32
; CSA_CHECK-NOT: sext32

entry:
  %p.addr = alloca float*, align 8
  store float* %p, float** %p.addr, align 8
  %0 = load float*, float** %p.addr, align 8
  %1 = load float, float* %0, align 4
  %sub = fsub float -0.000000e+00, %1
  ret float %sub
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}