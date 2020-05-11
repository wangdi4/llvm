; RUN: llc <%s | FileCheck %s
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "csa"

declare <4 x half> @llvm.fma.v4f16(<4 x half>, <4 x half>, <4 x half>)

define <4 x half> @addvec(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: addvec
; CHECK: addf16x4
  %res = fadd <4 x half> %a, %b
  ret <4 x half> %res
}

define <4 x half> @subvec(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: subvec
; CHECK: subf16x4
  %res = fsub <4 x half> %a, %b
  ret <4 x half> %res
}

define <4 x half> @mulvec(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: mulvec
; CHECK: mulf16x4
  %res = fmul <4 x half> %a, %b
  ret <4 x half> %res
}

define <4 x half> @divvec(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: divvec
; CHECK: unpack64_16
; CHECK: unpack64_16
; CHECK: divf16
; CHECK: divf16
; CHECK: divf16
; CHECK: divf16
; CHECK: pack16_64
  %res = fdiv <4 x half> %a, %b
  ret <4 x half> %res
}

define <4 x half> @fmavec(<4 x half> %a, <4 x half> %b, <4 x half> %c) {
; CHECK-LABEL: fmavec
; CHECK: fmaf16x4
  %res = call <4 x half> @llvm.fma.v4f16(<4 x half> %a, <4 x half> %b, <4 x half> %c)
  ret <4 x half> %res
}

define <4 x half> @asmvec(<4 x half> %a, <4 x half> %b, <4 x half> %c) {
; CHECK-LABEL: asmvec
; CHECK: fmsf16x4
  %res = call <4 x half> asm sideeffect "fmsf16x4 $0, $1, $2, $3, 3, 0, 0, 0", "=d,d,d,d"(<4 x half> %a, <4 x half> %b, <4 x half> %c)
  ret <4 x half> %res
}


define <4 x half> @shufflevector_a(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: shufflevector_a
; CHECK: shufi16x4
  %res = shufflevector <4 x half> %a, <4 x half> %b, <4 x i32> <i32 0, i32 4, i32 1, i32 3>
  ret <4 x half> %res
}

define <4 x half> @shufflevector_b(<4 x half> %a, <4 x half> %b) {
; CHECK-LABEL: shufflevector_b
; CHECK: shufi16x4
  %res = shufflevector <4 x half> %a, <4 x half> %b, <4 x i32> <i32 3, i32 1, i32 6, i32 7>
  ret <4 x half> %res
}

define <4 x half> @shufflevector_c(<4 x half> %a) {
; CHECK-LABEL: shufflevector_c
; CHECK: shufi16x4
  %res = shufflevector <4 x half> %a, <4 x half> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
  ret <4 x half> %res
}

define <4 x half> @swizzled_math(<4 x half> %apbi, <4 x half> %cpdi) {
; CHECK-LABEL: swizzled_math
; CHECK: .result .lic .i64 %[[RES:[a-zA-Z0-9_.]+]]
; CHECK: .param .lic .i64 %[[ARG0:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARG1:[a-zA-Z0-9_.]+]]
; CHECK: addf16x4 %[[RES]], %[[ARG0]], %[[ARG1]], 0, 1, 3
  %aa = shufflevector <4 x half> %apbi, <4 x half> undef, <4 x i32> <i32 0, i32 0, i32 2, i32 2>
  %a.shuffle = shufflevector <4 x half> undef, <4 x half> %cpdi, <4 x i32> <i32 5, i32 4, i32 7, i32 6>
  %res = fadd <4 x half> %aa, %a.shuffle
  ret <4 x half> %res
}

declare <4 x half> @llvm.csa.fmrsf16x4(<4 x half>, <4 x half>, <4 x half>, i8, i8, i8)

define <4 x half> @fmrs_intrinsic(<4 x half> %a, <4 x half> %b, <4 x half> %c) {
; CHECK-LABEL: fmrs_intrinsic
; CHECK: .result .lic .i64 %[[RES:[a-zA-Z0-9_.]+]]
; CHECK: .param .lic .i64 %[[ARGA:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARGB:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARGC:[a-zA-Z0-9_.]+]]
; CHECK: fmrsf16x4 %[[RES]], %[[ARGA]], %[[ARGB]], %[[ARGC]], 2, 3, 1
  %res = call <4 x half> @llvm.csa.fmrsf16x4(<4 x half> %a, <4 x half> %b, <4 x half> %c, i8 2, i8 3, i8 1)
  ret <4 x half> %res
}
