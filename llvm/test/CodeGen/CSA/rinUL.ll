; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i64 @rinUL(i64* %p) #0 {
; CSA_CHECK-LABEL: rinUL
; CSA_CHECK: neg64
; CSA_CHECK-NOT: sext32

entry:
  %p.addr = alloca i64*, align 8
  store i64* %p, i64** %p.addr, align 8
  %0 = load i64*, i64** %p.addr, align 8
  %1 = load i64, i64* %0, align 8
  %sub = sub i64 0, %1
  ret i64 %sub
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}