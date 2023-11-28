; RUN: opt < %s -passes=auto-cpu-clone -acd-enable-all -acd-force-no-funcs=foo -S | FileCheck %s
;
; The test verifies that if you use "-acd-force-no-funcs" as an argument with the compiler,
; all of the functions in a module should be multi-versioned except those which were
; specified with the option. -acd-enable-all is used to enable all functions, but it has
; lower priority, so even with this option, the specified functions should still be disabled.

; CHECK-LABEL: define dso_local void @foo(
; CHECK-SAME: ) #[[ATTR0:[0-9]+]] {
;
; CHECK-LABEL: define dso_local void @bar.A(
; CHECK-SAME: ) #[[ATTR0]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local void @bar.a(
; CHECK-SAME: ) #[[ATTR1:[0-9]+]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local ptr @bar.resolver(
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
