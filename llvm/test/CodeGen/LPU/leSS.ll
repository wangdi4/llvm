; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i32 @leSS(i16 signext %a, i16 signext %b) #0 {
LPU_CHECK-LABEL: leSS
LPU_CHECK: cmples32

entry:
  %a.addr = alloca i16, align 2
  %b.addr = alloca i16, align 2
  store i16 %a, i16* %a.addr, align 2
  store i16 %b, i16* %b.addr, align 2
  %0 = load i16* %a.addr, align 2
  %conv = sext i16 %0 to i32
  %1 = load i16* %b.addr, align 2
  %conv1 = sext i16 %1 to i32
  %cmp = icmp sle i32 %conv, %conv1
  %conv2 = zext i1 %cmp to i32
  ret i32 %conv2
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
