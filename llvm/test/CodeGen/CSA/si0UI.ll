; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @si0UI(i32* %p, i32** %p.addr) #0 {
; CSA_CHECK-LABEL: si0UI
; CSA_CHECK: st64
; CSA_CHECK: st32

entry:
  store i32* %p, i32** %p.addr, align 8
  %0 = load i32*, i32** %p.addr, align 8
  store i32 0, i32* %0, align 4
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
