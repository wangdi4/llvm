; RUN: llc -mtriple=csa < %s | FileCheck --implicit-check-not merge0 %s

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; We have had an issue in the past where i1 selects have been mistakenly lowered
; to merge0 ops instead of merge1 ones as a side-effect of our using i1 to
; represent 0-bit values at the IR level. This test ensures that i1 selects are
; correctly lowered to merge1.

define i1 @selecti1(i1 %cond, i1 %val1, i1 %val2) {
; CHECK-LABEL: selecti1
; CHECK: merge1

  %result = select i1 %cond, i1 %val1, i1 %val2
  ret i1 %result
}
