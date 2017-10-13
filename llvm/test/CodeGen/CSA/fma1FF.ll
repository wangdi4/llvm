; RUN: llc -fp-contract=fast -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define float @fma1FF(float %a, float %b, float %c) #0 {
; CSA_CHECK-LABEL: fma1FF
; CSA_CHECK: fmaf32

entry:
  %a.addr = alloca float, align 4
  %b.addr = alloca float, align 4
  %c.addr = alloca float, align 4
  store float %a, float* %a.addr, align 4
  store float %b, float* %b.addr, align 4
  store float %c, float* %c.addr, align 4
  %0 = load float, float* %a.addr, align 4
  %1 = load float, float* %b.addr, align 4
  %mul = fmul float %0, %1
  %2 = load float, float* %c.addr, align 4
  %add = fadd float %mul, %2
  ret float %add
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
