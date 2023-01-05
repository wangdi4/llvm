; RUN: opt -opaque-pointers -passes=auto-cpu-clone -enable-selective-mv=0 < %s -S | FileCheck %s

; The test checks that functions that are redefinitions of library defined functions,
; are not multiversioned.

; CHECK: define dso_local ptr @strcpy
; CHECK-NOT: @strcpy.A
; CHECK-NOT: @strcpy.a
; CHECK-NOT: @strcpy.resolver

; CHECK: define dso_local i32 @strcmp
; CHECK-NOT: @strcmp.A
; CHECK-NOT: @strcmp.a
; CHECK-NOT: @strcmp.resolver

; CHECK: define dso_local i64 @strlen
; CHECK-NOT: @strlen.A
; CHECK-NOT: @strlen.a
; CHECK-NOT: @strlen.resolver

; CHECK-NOT: define dso_local void @reverse(
; CHECK-DAG: @reverse.A
; CHECK-DAG: @reverse.a
; CHECK-DAG: @reverse.resolver

; CHECK-NOT: define dso_local void @main
; CHECK-DAG: @main.A
; CHECK-DAG: @main.a
; CHECK-DAG: @main.resolver


; ModuleID = '<stdin>'
source_filename = "<stdin>"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local ptr @strcpy(ptr noundef %s, ptr noundef %t) !llvm.auto.cpu.dispatch !3 {
entry:
  ret ptr %s
}

define dso_local i32 @strcmp(ptr noundef %s, ptr noundef %t) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 0
}

define dso_local i64 @strlen(ptr noundef %s) !llvm.auto.cpu.dispatch !3 {
entry:
  ret i64 0
}

define dso_local void @reverse(ptr noundef %s) !llvm.auto.cpu.dispatch !3 {
entry:
  ret void
}

define dso_local i32 @main() !llvm.auto.cpu.dispatch !3 {
entry:
  ret i32 0
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!""}
!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
