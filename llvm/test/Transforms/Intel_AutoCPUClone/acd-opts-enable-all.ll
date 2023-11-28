; RUN: opt < %s -passes=auto-cpu-clone -acd-enable-all -S | FileCheck %s
; 
; The test chack that all functions in a module were multiversioned if
; -acd-enable-all passed to compiler
;
; CHECK-LABEL: define dso_local void @foo.A(
; CHECK-SAME: ) #[[ATTR0:[0-9]+]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local void @bar.A(
; CHECK-SAME: ) #[[ATTR0]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local void @foo.a(
; CHECK-SAME: ) #[[ATTR1:[0-9]+]] !llvm.acd.clone !0 {
;
; CHECK-LABEL: define dso_local ptr @foo.resolver(
; CHECK-SAME: ) #[[ATTR0]] {
;
; CHECK-LABEL: define dso_local void @bar.a(
; CHECK-SAME: ) #[[ATTR1]] !llvm.acd.clone !0 {
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
