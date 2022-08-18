; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; f3_a -> f3_b -> f3_c
@f3_b = dso_local alias i8* (...), bitcast (i8* ()* @f3_c to i8* (...)*)
@f3_a = dso_local ifunc i8* (...), bitcast (i8* (...)* @f3_b to i8* (...)* ()*)

; CHECK: ^[[L1:[0-9]]] = gv: (name: "f3_c",
; CHECK-SAME: live: 1
; CHECK: gv: (name: "f3_b", {{.*}} aliasee: ^[[L1]])

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i8* @f3_c() {
entry:
  ret i8* null
}
