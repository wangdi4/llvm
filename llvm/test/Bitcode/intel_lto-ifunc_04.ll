; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; CHECK: ^[[L1:[0-9]]] = gv: (name: "MCD.ifunc2", {{.*}} aliasee: ^[[L4:[0-9]]])
; CHECK: gv: (name: "MCD.ifunc1", {{.*}} aliasee: ^[[L1]])
; CHECK: gv: (name: "MCD", {{.*}} aliasee: ^[[L1]])
; CHECK: ^[[L4]] = gv: (name: "MCD.resolver",

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
