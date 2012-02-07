; RUN: llc -mcpu=corei7-avx < %s

define <4 x i32> @trunc_64_32(<4 x i64> %A) nounwind {
; CHECK: trunc_64_32
; CHECK: pshufd
  %B = trunc <4 x i64> %A to <4 x i32>
  ret <4 x i32>%B
}
define <8 x i16> @trunc_32_16(<8 x i32> %A) nounwind {
; CHECK: trunc_32_16
; CHECK: pshufb
  %B = trunc <8 x i32> %A to <8 x i16>
  ret <8 x i16>%B
}

