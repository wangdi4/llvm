; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define zeroext i16 @rinUS(i16* %p) #0 {
; CSA_CHECK-LABEL: rinUS
; CSA_CHECK: neg16
; CSA_CHECK-NOT: sext32

entry:
  %p.addr = alloca i16*, align 8
  store i16* %p, i16** %p.addr, align 8
  %0 = load i16*, i16** %p.addr, align 8
  %1 = load i16, i16* %0, align 2
  %sub = sub nsw i16 0, %1
  ret i16 %sub
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}