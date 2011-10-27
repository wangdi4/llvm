; RUN: llc -mcpu=sandybridge < %s

define <4 x i64> @add_4_i64(<4 x i64> %broadcast2) nounwind readnone {
; CHECK: add_4_i64
; CHECK: vextractf128
; CHECK: vpaddq
; CHECK: vinsertf128
  %1 = add <4 x i64> %broadcast2, <i64 0, i64 0, i64 0, i64 0>
  ret <4 x i64> %1
  }
define <8 x i32> @add_8_i32(<8 x i32> %broadcast2) nounwind readnone {
; CHECK: add_8_i32
; CHECK: vpaddd
  %1 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %1
  }

define <32 x i8> @add_32_i8(<32 x i8> %broadcast2) nounwind readnone {
; CHECK: add_32_i8
; CHECK: vpaddb
  %1 = add <32 x i8> %broadcast2, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3 >
  ret <32 x i8> %1
  }

define <16 x i16> @add_16_i16(<16 x i16> %broadcast2) nounwind readnone {
; CHECK: add_16_i16
; CHECK: vpaddw
  %1 = add <16 x i16> %broadcast2, <i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 8, i16 9, i16 2, i16 3>
  ret <16 x i16> %1
  }

