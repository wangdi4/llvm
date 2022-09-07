; RUN: opt -opaque-pointers -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; If we multiversion internal function, all the auto-generated symbols must have
; the same linkage so that multiversioned symbols from different TUs don't
; clash.


; CHECK-DAG: @llvm.compiler.used = {{.*}} @bar
; CHECK-DAG: @foo = internal ifunc i32 (i32), ptr @foo.resolver
; CHECK-DAG: define internal ptr @foo.resolver()
; CHECK-DAG: define internal i32 @foo.A(i32 %a)
; CHECK-DAG: define internal i32 @foo.P(i32 %a)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @foo(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %add = add i32 %a, 42
  ret i32 %add
}

define i32 @bar(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %ret = call i32 @foo(i32 42)
  ret i32 %ret
}

!0 = !{!1}
!1 = !{!"auto-cpu-dispatch-target", !"core_i7_sse4_2"}
