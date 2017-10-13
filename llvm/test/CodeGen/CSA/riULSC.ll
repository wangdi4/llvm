; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @riULSC(i8* %p) #0 {
; CSA_CHECK-LABEL: riULSC
; CSA_CHECK: sext64

entry:
  %p.addr = alloca i8*, align 8
  store i8* %p, i8** %p.addr, align 8
  %0 = load i8*, i8** %p.addr, align 8
  %1 = load i8, i8* %0, align 1
  %conv = sext i8 %1 to i64
  ret i64 %conv
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}