; RUN: llc -fp-contract=fast -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define float @fmapFF(float %a, float %b, float %c, float %d) #0 {
; CSA_CHECK-LABEL: fmapFF
; CSA_CHECK: fmaf32

entry:
  %mul = fmul float %a, %b
  %add = fadd float %mul, %c
  %add1 = fadd float %add, %d
  ret float %add1
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
