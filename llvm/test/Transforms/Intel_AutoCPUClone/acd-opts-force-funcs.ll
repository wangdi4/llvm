; RUN: opt < %s -passes=auto-cpu-clone -acd-disable-all -acd-force-funcs=foo -S | FileCheck %s
;
; The test verifies that if you use "-acd-force-funcs" as an argument with the compiler,
; only the requested functions in a module should be multi-versioned. -acd-disable-all
; is used to disable all functions, but it has lower priority, so even with this option,
; the functions specified with -acd-force-funcs functions should still be multi-versioned.

; CHECK-LABEL: define dso_local void @foo.A(
; CHECK-SAME: ) #[[ATTR0:[0-9]+]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local void @bar(
; CHECK-SAME: ) #[[ATTR0]] {
;
; CHECK-LABEL: define dso_local void @foo.a(
; CHECK-SAME: ) #[[ATTR1:[0-9]+]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local ptr @foo.resolver(
; CHECK-SAME: ) #[[ATTR0]] {


source_filename = "t1.cpp"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() !llvm.auto.cpu.dispatch !0 {
entry:
  ret void
}

define dso_local void @bar() !llvm.auto.cpu.dispatch !0 {
entry:
  ret void
}

!0 = !{!"skylake-avx512"}
