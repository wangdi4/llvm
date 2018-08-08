; RUN: llvm-link %CSABASE_ROOT/libcsa/libcsac.bc %s -o %t
; RUN: llc -mtriple=csa < %t | FileCheck %s --check-prefix=CSA_CHECK
; RUN: rm %t

; ModuleID = 'tools/src/llvm/test/CodeGen/CSA/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define i32 @first(i32 %a, i32 %b, i16* %c, i32* %ip) #0 {
; CSA_CHECK-LABEL: first
; CSA_CHECK: ld32x

entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %c.addr = alloca i16*, align 8
  %ip.addr = alloca i32*, align 8
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  store i16* %c, i16** %c.addr, align 8
  store i32* %ip, i32** %ip.addr, align 8
  %0 = load i32, i32* %a.addr, align 4
  %1 = load i32, i32* %b.addr, align 4
  %mul = mul nsw i32 %0, %1
  %2 = load i32, i32* %a.addr, align 4
  %add = add nsw i32 %2, 2
  %idxprom = sext i32 %add to i64
  %3 = load i32*, i32** %ip.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %3, i64 %idxprom
  %4 = load i32, i32* %arrayidx, align 4
  %add1 = add nsw i32 %mul, %4
  %5 = load i16*, i16** %c.addr, align 8
  %6 = load i16, i16* %5, align 2
  %conv = sext i16 %6 to i32
  %add2 = add nsw i32 %add1, %conv
  ret i32 %add2
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
