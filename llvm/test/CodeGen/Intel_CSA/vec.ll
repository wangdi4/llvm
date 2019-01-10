; RUN: llc <%s | FileCheck %s
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "csa"

declare <2 x float> @llvm.fma.v2f32(<2 x float>, <2 x float>, <2 x float>)

define <2 x float> @addvec(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: addvec
; CHECK: addf32x2
  %res = fadd <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @subvec(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: subvec
; CHECK: subf32x2
  %res = fsub <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @mulvec(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: mulvec
; CHECK: mulf32x2
  %res = fmul <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @divvec(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: divvec
; CHECK: unpack64_32
; CHECK: unpack64_32
; CHECK: divf32
; CHECK: divf32
; CHECK: pack32_64
  %res = fdiv <2 x float> %a, %b
  ret <2 x float> %res
}

define <2 x float> @fmavec(<2 x float> %a, <2 x float> %b, <2 x float> %c) {
; CHECK-LABEL: fmavec
; CHECK: fmaf32x2
  %res = call <2 x float> @llvm.fma.v2f32(<2 x float> %a, <2 x float> %b, <2 x float> %c)
  ret <2 x float> %res
}

define <2 x float> @asmvec(<2 x float> %a, <2 x float> %b, <2 x float> %c) {
; CHECK-LABEL: asmvec
; CHECK: fmsf32x2
  %res = call <2 x float> asm sideeffect "fmsf32x2 $0, $1, $2, $3, 3, 0, 0, 0", "=d,d,d,d"(<2 x float> %a, <2 x float> %b, <2 x float> %c)
  ret <2 x float> %res
}


define <2 x float> @shufflevector_a(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: shufflevector_a
; CHECK: shufi32x2
  %res = shufflevector <2 x float> %a, <2 x float> %b, <2 x i32> <i32 0, i32 2>
  ret <2 x float> %res
}

define <2 x float> @shufflevector_b(<2 x float> %a, <2 x float> %b) {
; CHECK-LABEL: shufflevector_b
; CHECK: shufi32x2
  %res = shufflevector <2 x float> %a, <2 x float> %b, <2 x i32> <i32 3, i32 1>
  ret <2 x float> %res
}

define <2 x float> @shufflevector_c(<2 x float> %a) {
; CHECK-LABEL: shufflevector_c
; CHECK: shufi32x2
  %res = shufflevector <2 x float> %a, <2 x float> undef, <2 x i32> <i32 1, i32 0>
  ret <2 x float> %res
}

define <2 x float> @swizzled_math(<2 x float> %apbi, <2 x float> %cpdi) {
; CHECK-LABEL: swizzled_math
; CHECK: .result .lic .i64 %[[RES:[a-zA-Z0-9_.]+]]
; CHECK: .param .lic .i64 %[[ARG0:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARG1:[a-zA-Z0-9_.]+]]
; CHECK: addf32x2 %[[RES]], %[[ARG0]], %[[ARG1]], 0, 1, 3
  %aa = shufflevector <2 x float> %apbi, <2 x float> undef, <2 x i32> <i32 0, i32 0>
  %a.shuffle = shufflevector <2 x float> undef, <2 x float> %cpdi, <2 x i32> <i32 3, i32 2>
  %res = fadd <2 x float> %aa, %a.shuffle
  ret <2 x float> %res
}

declare <2 x float> @llvm.csa.fmrsf32x2(<2 x float>, <2 x float>, <2 x float>, i8, i8, i8)

define <2 x float> @fmrs_intrinsic(<2 x float> %a, <2 x float> %b, <2 x float> %c) {
; CHECK-LABEL: fmrs_intrinsic
; CHECK: .result .lic .i64 %[[RES:[a-zA-Z0-9_.]+]]
; CHECK: .param .lic .i64 %[[ARGA:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARGB:[a-zA-Z0-9_.]+]]
; CHECK-NEXT: .param .lic .i64 %[[ARGC:[a-zA-Z0-9_.]+]]
; CHECK: fmrsf32x2 %[[RES]], %[[ARGA]], %[[ARGB]], %[[ARGC]], 2, 3, 1
  %res = call <2 x float> @llvm.csa.fmrsf32x2(<2 x float> %a, <2 x float> %b, <2 x float> %c, i8 2, i8 3, i8 1)
  ret <2 x float> %res
}
