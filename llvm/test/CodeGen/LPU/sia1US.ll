; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK 

; ModuleID = 'tools/src/llvm/test/CodeGen/LPU/ALUOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define void @sia1US(i16* %p) #0 {
LPU_CHECK-LABEL: sia1US
LPU_CHECK: st64
LPU_CHECK: ld16
LPU_CHECK: st16

entry:
  %p.addr = alloca i16*, align 8
  store i16* %p, i16** %p.addr, align 8
  %0 = load i16** %p.addr, align 8
  %1 = load i16* %0, align 2
  %conv = zext i16 %1 to i32
  %add = add nsw i32 %conv, 1
  %conv1 = trunc i32 %add to i16
  store i16 %conv1, i16* %0, align 2
  ret void
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
