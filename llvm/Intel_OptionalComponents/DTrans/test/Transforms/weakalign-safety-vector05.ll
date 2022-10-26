; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign  -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; vector instructions. This case checks the 'insertelement' instruction.

; CHECK: DTRANS Weak Align: inhibited -- Unsupported vector instruction

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Corresponds to call to: _mm_insert_epi64
define internal <2 x i64> @test01(<2 x i64> %x, i64 %y) {
  %z = insertelement <2 x i64> %x, i64 %y, i64 1
  ret <2 x i64> %z
}

define i32 @main() {
  ret i32 0
}
