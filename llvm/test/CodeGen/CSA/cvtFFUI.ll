; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define float @cvtFFUI(i32 %u) #0 {
; CSA_CHECK-LABEL: cvtFFUI
; CSA_CHECK: cvtf32u32

entry:
  %u.addr = alloca i32, align 4
  store i32 %u, i32* %u.addr, align 4
  %0 = load i32, i32* %u.addr, align 4
  %conv = uitofp i32 %0 to float
  ret float %conv
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}