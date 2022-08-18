; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; CHECK: gv: (name: "MCD.ifunc1"
; CHECK-NOT:  summaries: (
; CHECK-SAME: ; guid = {{[0-9]+}}
; CHECK: gv: (name: "MCD"
; CHECK-NOT:  summaries: (
; CHECK-SAME: ; guid = {{[0-9]+}}
; CHECK: gv: (name: "MCD.resolver",
; CHECK-SAME: live: 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; MCD (alias) -> MCD.ifunc1 (alias) -> MCD.ifunc2 (ifunc) -> MCD.resolver (function)
;
@MCD = weak_odr dso_local alias void (), void ()* @MCD.ifunc1
@MCD.ifunc1 = weak_odr dso_local alias void (), void ()* @MCD.ifunc2
@MCD.ifunc2 = weak_odr dso_local ifunc void (), void ()* ()* @MCD.resolver

define weak_odr void ()* @MCD.resolver() {
entry:
  ret void ()* null
}
