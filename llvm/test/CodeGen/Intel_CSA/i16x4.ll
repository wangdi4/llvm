; RUN: llc <%s | FileCheck %s
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "csa"

define <4 x i16> @addvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: addvec
; CHECK: add16x4
  %res = add <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @subvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: subvec
; CHECK: sub16x4
  %res = sub <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @mulvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: mulvec
; CHECK: unpack64_16
; CHECK: unpack64_16
; CHECK: mul16
; CHECK: mul16
; CHECK: mul16
; CHECK: mul16
; CHECK: pack16_64
  %res = mul <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @divvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: divvec
; CHECK: unpack64_16
; CHECK: unpack64_16
; CHECK: divu16
; CHECK: divu16
; CHECK: divu16
; CHECK: divu16
; CHECK: pack16_64
  %res = udiv <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @andvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: andvec
; CHECK: and64
  %res = and <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @orvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: orvec
; CHECK: or64
  %res = or <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @xorvec(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: xorvec
; CHECK: xor64
  %res = xor <4 x i16> %a, %b
  ret <4 x i16> %res
}

define <4 x i16> @shufflevector_a(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: shufflevector_a
; CHECK: shufi16x4
  %res = shufflevector <4 x i16> %a, <4 x i16> %b, <4 x i32> <i32 0, i32 4, i32 1, i32 3>
  ret <4 x i16> %res
}

define <4 x i16> @shufflevector_b(<4 x i16> %a, <4 x i16> %b) {
; CHECK-LABEL: shufflevector_b
; CHECK: shufi16x4
  %res = shufflevector <4 x i16> %a, <4 x i16> %b, <4 x i32> <i32 3, i32 1, i32 6, i32 7>
  ret <4 x i16> %res
}

define <4 x i16> @shufflevector_c(<4 x i16> %a) {
; CHECK-LABEL: shufflevector_c
; CHECK: shufi16x4
  %res = shufflevector <4 x i16> %a, <4 x i16> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
  ret <4 x i16> %res
}

