; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
; ModuleID = '<stdin>'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind readnone
define i32 @popci(i32 %i) #0 {
; CSA_CHECK-LABEL: popci
; CSA_CHECK: ctpop32
entry:
  %0 = tail call i32 @llvm.ctpop.i32(i32 %i)
  ret i32 %0
}

; Function Attrs: nounwind readnone
declare i32 @llvm.ctpop.i32(i32) #1

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
