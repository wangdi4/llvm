; RUN: llc < %s | FileCheck %s
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "csa"

declare <2 x float> @llvm.fma.v2f32(<2 x float>, <2 x float>, <2 x float>)

define <2 x float> @addvec(<2 x float> %a, <2 x float> %b) {
; CHECK: addvec
; CHECK: addf32x2
  %res = fadd <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @subvec(<2 x float> %a, <2 x float> %b) {
; CHECK: subvec
; CHECK: subf32x2
  %res = fsub <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @mulvec(<2 x float> %a, <2 x float> %b) {
; CHECK: mulvec
; CHECK: mulf32x2
  %res = fmul <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @divvec(<2 x float> %a, <2 x float> %b) {
; CHECK: divvec
; CHECK: unpack64_32
; CHECK: divf32
; CHECK: divf32
; CHECK: pack32_64
  %res = fdiv <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @fmavec(<2 x float> %a, <2 x float> %b, <2 x float> %c) {
; CHECK: fmavec
; CHECK: fmaf32x2
  %res = call <2 x float> @llvm.fma.v2f32(<2 x float> %a, <2 x float> %b, <2 x float> %c)
  ret <2 x float> %res
}
