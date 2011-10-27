; RUN: llc -mcpu=haswell < %s

define <4 x i64> @and_4_i64(<4 x i64> %A) nounwind readnone {
; CHECK: and_4_i64
; CHECK-NOT: extract
; CHECK: vpand
; CHECK-NOT: insert
  %1 = and <4 x i64> %A, <i64 0, i64 1, i64 0, i64 0>
  ret <4 x i64> %1
  }
define <8 x i32> @and_8_i32(<8 x i32> %A) nounwind readnone {
; CHECK: and_8_i32
; CHECK: vpand
  %1 = and <8 x i32> %A, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %1
  }

define <32 x i8> @and_32_i8(<32 x i8> %A) nounwind readnone {
; CHECK: and_32_i8
; CHECK: vpand
  %1 = and <32 x i8> %A, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3 >
  ret <32 x i8> %1
  }

define <16 x i16> @or_16_i16(<16 x i16> %A) nounwind readnone {
; CHECK: or_16_i16
; CHECK-NOT: insert
; CHECK: vpor
  %1 = or <16 x i16> %A, <i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 8, i16 9, i16 2, i16 3>
  ret <16 x i16> %1
  }

define <4 x i64> @xor_4_i64(<4 x i64> %A) nounwind readnone {
; CHECK: xor_4_i64
; CHECK-NOT: extract
; CHECK: vpxor
; CHECK-NOT: insert
  %1 = xor <4 x i64> %A, <i64 0, i64 7, i64 0, i64 0>
  ret <4 x i64> %1
  }
define <8 x i32> @xor_8_i32(<8 x i32> %A) nounwind readnone {
; CHECK: xor_8_i32
; CHECK: vpxor
  %1 = xor <8 x i32> %A, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %1
  }

define <32 x i8> @xor_32_i8(<32 x i8> %A) nounwind readnone {
; CHECK: xor_32_i8
; CHECK: vpxor
  %1 = xor <32 x i8> %A, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3 >
  ret <32 x i8> %1
  }

define <16 x i16> @xor_16_i16(<16 x i16> %A) nounwind readnone {
; CHECK: xor_16_i16
; CHECK-NOT: insert
; CHECK: vpxor
  %1 = xor <16 x i16> %A, <i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 8, i16 9, i16 2, i16 3>
  ret <16 x i16> %1
  }

