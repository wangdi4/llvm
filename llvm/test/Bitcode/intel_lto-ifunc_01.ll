; CMPLRLLVM-11512: Check ifunc is handled while computing module summary.
; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; CHECK: ^[[L1:[0-9]]] = gv: (name: "MCD.ifunc", {{.*}}  aliasee: ^[[L3:[0-9]]]
; CHECK: gv: (name: "MCD", {{.*}} aliasee: ^[[L1]]
; CHECK: ^[[L3]] = gv: (name: "MCD.resolver",

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@MCD = weak_odr dso_local alias void (), void ()* @MCD.ifunc
@MCD.ifunc = weak_odr dso_local ifunc void (), void ()* ()* @MCD.resolver

define weak_odr void ()* @MCD.resolver() {
entry:
  ret void ()* null
}
