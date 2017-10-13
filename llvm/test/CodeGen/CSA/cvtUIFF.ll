; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @cvtUIFF(float %f) #0 {
; CSA_CHECK-LABEL: cvtUIFF
; CSA_CHECK: cvtu32f32

entry:
  %f.addr = alloca float, align 4
  store float %f, float* %f.addr, align 4
  %0 = load float, float* %f.addr, align 4
  %conv = fptoui float %0 to i32
  ret i32 %conv
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}