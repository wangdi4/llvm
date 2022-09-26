; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; vector instructions. This case checks the 'shufflevector' instruction.

; CHECK: DTRANS Weak Align: inhibited -- Unsupported vector instruction

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Corresponds to call to: _mm256_unpackhi_epi16
define internal <4 x i64> @test01(<4 x i64> %a, <4 x i64> %b) {
  %a.vec = bitcast <4 x i64> %a to <16 x i16>
  %b.vec = bitcast <4 x i64> %b to <16 x i16>
  %shuffle.i = shufflevector <16 x i16> %a.vec, <16 x i16> %b.vec, <16 x i32> <i32 4, i32 20, i32 5, i32 21, i32 6, i32 22, i32 7, i32 23, i32 12, i32 28, i32 13, i32 29, i32 14, i32 30, i32 15, i32 31>
  %c.vec = bitcast <16 x i16> %shuffle.i to <4 x i64>
  ret <4 x i64> %c.vec
}

define i32 @main() {
  ret i32 0
}
