; RUN: opt -passes=auto-cpu-clone -acd-enable-all < %s -S | FileCheck %s

; This test checks that functions are multiversioned using wrapper based resolvers
; when "PIC Level" is present among the module flags and the flag's value is different
; from PICLevel::NotPIC.


; CHECK:     @foo.ptr = internal global ptr @foo.A, !llvm.acd.dispatcher !0
; CHECK-DAG: define void @foo.A() #0 !llvm.acd.clone !0 {
; CHECK-DAG: define void @foo.V() #1 !llvm.acd.clone !0 {
; CHECK-DAG: define internal void @__intel.acd.resolver() #0 {
; CHECK-DAG: define void @foo() #0 !llvm.acd.dispatcher !0 {


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo() !llvm.auto.arch !0 {
  ret void
}

!llvm.module.flags = !{!1}

!0 = !{!"haswell"}
!1 = !{i32 8, !"PIC Level", i32 1}
