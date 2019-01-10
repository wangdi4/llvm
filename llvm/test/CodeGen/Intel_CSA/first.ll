; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @first(i32 %a, i32 %b, i16* %c, i32* %ip) #0 {
; CSA_CHECK-LABEL: first
; CSA_CHECK: add64
; CSA_CHECK: ld32

entry:
  %mul = mul nsw i32 %a, %b
  %add = add nsw i32 %a, 2
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add1 = add nsw i32 %mul, %0
  %1 = load i16, i16* %c, align 2
  %conv = sext i16 %1 to i32
  %add2 = add nsw i32 %add1, %conv
  ret i32 %add2
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
