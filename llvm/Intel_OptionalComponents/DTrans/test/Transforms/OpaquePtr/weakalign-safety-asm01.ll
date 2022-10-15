; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of an
; inline asm.

; CHECK: DTRANS Weak Align: inhibited -- Contains inline asm

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Uses inline asm
define internal void @test01() {
  call void asm "nop", ""()
 ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

!intel.dtrans.types = !{}

