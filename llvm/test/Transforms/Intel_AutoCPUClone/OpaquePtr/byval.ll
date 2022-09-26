; RUN: opt -opaque-pointers -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; Trying to add byval to the ifunc would result in
;
;   error: argument attributes invalid in function type
;
; So that *seems* to be fine.

; CHECK-DAG: @foo = ifunc i32 (ptr), ptr @foo.resolver
; CHECK-DAG: define i32 @foo.A(ptr byval(%struct) %s) #{{[0-9]*}} !llvm.acd.clone !0 {
; CHECK-DAG: define i32 @foo.b(ptr byval(%struct) %s) #{{[0-9]*}} !llvm.acd.clone !0 {
; CHECK-DAG: define ptr @foo.resolver()
; CHECK-DAG: ret ptr @foo.b
; CHECK-DAG: ret ptr @foo.A


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct = type { i32 }

define i32 @foo(ptr byval(%struct) %s) !llvm.auto.cpu.dispatch !0 {
  %gep = getelementptr %struct, ptr %s, i32 0, i32 0
  %f = load i32, ptr %gep, align 4
  %add = add i32 %f, 42
  ret i32 %add
}

define i32 @bar(i32 %a) {
  %s = alloca %struct, align 8
  %gep = getelementptr %struct, ptr %s, i32 0, i32 0
  store i32 %a, ptr %gep, align 4
  %ret = call i32 @foo(ptr byval(%struct) %s)
  ret i32 %ret
}

!0 = !{!1}
!1 = !{!"auto-cpu-dispatch-target", !"skylake"}
