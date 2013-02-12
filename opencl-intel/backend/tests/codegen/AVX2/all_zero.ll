; RUN: llc -mcpu=haswell < %s

define <4 x i64> @add_4_i64(<4 x i64> %A) nounwind readnone {
; CHECK: add_4_i64
; CHECK: vpxor
  %1 = add <4 x i64> %A, <i64 0, i64 0, i64 0, i64 0>
  ret <4 x i64> %1
}

define <8 x i32> @add_8_i32(<8 x i32> %A) nounwind readnone {
; CHECK: add_8_i32
; CHECK: vpxor
  %1 = add <8 x i32> %A, <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>
  ret <8 x i32> %1
  }
  
define <16 x i16> @add_16_i16(<16 x i16> %A) nounwind readnone {
; CHECK: add_16_i16
; CHECK: vpxor
  %1 = add <16 x i16> %A, <i16 0, i16 0, i16 undef, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0, i16 0>
  ret <16 x i16> %1
  }
