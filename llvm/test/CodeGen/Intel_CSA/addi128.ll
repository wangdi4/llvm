; RUN: llc -mtriple=csa < %s | FileCheck %s

; ModuleID = 'MathOps.c'
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define i128 @addi128(i128 %a, i128 %b) {
; CHECK-LABEL: addi128
; CHECK: adc64 %[[lo:[A-Za-z0-9_.]+]], %[[carry:[A-Za-z0-9_.]+]], %[[a_lo:[A-Za-z0-9_.]+]], %[[b_lo:[A-Za-z0-9_.]+]]
; CHECK: adc64 %[[hi:[A-Za-z0-9_.]+]], %ign, %[[a_hi:[A-Za-z0-9_.]+]], %[[b_hi:[A-Za-z0-9_.]+]], %[[carry]]
entry:
  %res = add i128 %a, %b
  ret i128 %res
}

define i128 @subi128(i128 %a, i128 %b) {
; CHECK-LABEL: subi128
; CHECK: sbb64 %[[lo:[A-Za-z0-9_.]+]], %[[carry:[A-Za-z0-9_.]+]], %[[a_lo:[A-Za-z0-9_.]+]], %[[b_lo:[A-Za-z0-9_.]+]]
; CHECK: sbb64 %[[hi:[A-Za-z0-9_.]+]], %ign, %[[a_hi:[A-Za-z0-9_.]+]], %[[b_hi:[A-Za-z0-9_.]+]], %[[carry]]
entry:
  %res = sub i128 %a, %b
  ret i128 %res
}
