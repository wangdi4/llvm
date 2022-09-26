; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; vector instructions. This case checks the 'extractelement' instruction.

; CHECK: DTRANS Weak Align: inhibited -- Unsupported vector instruction

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Corresponds to call to: _mm_extract_ps
define internal i32 @test01(<4 x i32> %a) {
  %b = extractelement <4 x i32> %a, i64 1
  ret i32 %b
}

define i32 @main() {
  ret i32 0
}
