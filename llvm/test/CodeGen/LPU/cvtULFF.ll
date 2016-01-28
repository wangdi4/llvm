; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i64 @cvtULFF(float %f) #0 {
LPU_CHECK-LABEL: cvtULFF
LPU_CHECK: cvtu64f32

entry:
  %f.addr = alloca float, align 4
  store float %f, float* %f.addr, align 4
  %0 = load float* %f.addr, align 4
  %conv = fptoui float %0 to i64
  ret i64 %conv
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}