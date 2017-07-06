; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @si0SS(i16* %p, i16** %p.addr) #0 {
; CSA_CHECK-LABEL: si0SS
; CSA_CHECK: st64
; CSA_CHECK: st16
entry:
  store i16* %p, i16** %p.addr, align 8
  %0 = load i16*, i16** %p.addr, align 8
  store i16 0, i16* %0, align 2
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
