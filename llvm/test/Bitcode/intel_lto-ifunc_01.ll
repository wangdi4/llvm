; CMPLRLLVM-11512: Check ifunc is handled while computing module summary.
; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; CHECK:      (name: "MCD"
; CHECK-NOT:  summaries: (
; CHECK-SAME: ; guid = {{[0-9]+}}
; CHECK: (name: "MCD.resolver",
; CHECK-SAME: live: 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@MCD = weak_odr dso_local alias void (), void ()* @MCD.ifunc
@MCD.ifunc = weak_odr dso_local ifunc void (), void ()* ()* @MCD.resolver

define weak_odr void ()* @MCD.resolver() {
entry:
  ret void ()* null
}
