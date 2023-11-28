; RUN: opt < %s -passes=auto-cpu-clone -acd-enable-all -acd-disable-all -S | FileCheck %s
;
; The test verifies that if you use "-acd-disable-all" as an argument with the compiler,
; none of the functions in a module should be multi-versioned. -acd-enable-all is used
; to enable all functions, but it has lower priority, so even with this option,
; all functions should still be disabled.

; CHECK-LABEL: define dso_local void @foo() {
; CHECK-LABEL: define dso_local void @bar() {

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
