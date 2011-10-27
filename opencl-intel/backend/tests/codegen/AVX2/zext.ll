; RUN: llc -mcpu=haswell < %s


define <8 x i32> @zext_8i16_to_8i32(<8 x i16> %A) nounwind {
;CHECK: zext_8i16_to_8i32
;CHECK: vpmovzxwd
;CHECK-NOT: vpunpckhwd

  %B = zext <8 x i16> %A to <8 x i32>
  ret <8 x i32>%B
}

define <4 x i64> @zext_4i32_to_4i64(<4 x i32> %A) nounwind {
;CHECK: zext_4i32_to_4i64
;CHECK: vpmovzxdq
;CHECK-NOT: vpunpckhdq

  %B = zext <4 x i32> %A to <4 x i64>
  ret <4 x i64>%B
}

