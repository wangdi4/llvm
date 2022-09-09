; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited if the target does not
; have the mallopt library call.

; CHECK: DTRANS Weak Align: inhibited -- mallopt() not available

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-win32-elf"

define i32 @main() {
  ret i32 0
}
