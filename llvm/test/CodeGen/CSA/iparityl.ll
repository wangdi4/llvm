; RUN: llc -mtriple=csa < %s | FileCheck %s --check-prefix=CSA_CHECK
; ModuleID = '<stdin>'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind readnone
define i64 @iparityl(i64 %i) #0 {
; CSA_CHECK-LABEL: iparityl
; CSA_CHECK: parity64
entry:
  %0 = tail call i64 @llvm.ctpop.i64(i64 %i)
  %and = and i64 %0, 1
  ret i64 %and
}

; Function Attrs: nounwind readnone
declare i64 @llvm.ctpop.i64(i64) #1

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.6.0 (tags/RELEASE_360/final)"}
