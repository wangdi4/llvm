; RUN: llc -mtriple=lpu < %s | FileCheck %s --check-prefix=LPU_CHECK
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "lpu"

; Function Attrs: nounwind
define i32 @f_atmand32(i32* %m, i32 signext %v) #0 {
; LPU_CHECK-LABEL: f_atmand32
; LPU_CHECK: atmand32
entry:
  %0 = atomicrmw and i32* %m, i32 %v seq_cst
  ret i32 %0
}

