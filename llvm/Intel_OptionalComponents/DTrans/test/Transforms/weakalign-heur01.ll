; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=false -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=false -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited if the heuristic
; that checks for an SOA-to-AOS function fails to find a routine.

; CHECK: DTRANS Weak Align: inhibited -- Did not find SOA-to-AOS transformed routine

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; There is no function marked with the SOA-to-AOS transformation
; metadata, so the weak align pass will be inhibited.
define internal void @test01() {
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}
