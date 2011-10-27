; RUN: llc -mcpu=sandybridge < %s

define <8 x i32> @zext_8i8_to_8i32(<8 x i8> %A) nounwind {
;CHECK: zext_8i8_to_8i32
;CHECK: vpmovzxbw
;CHECK: vpunpckhwd

  %B = zext <8 x i8> %A to <8 x i32>
  ret <8 x i32>%B
}

define <8 x i32> @zext_8i16_to_8i32(<8 x i16> %A) nounwind {
;CHECK: zext_8i16_to_8i32
;CHECK: vpunpckhwd

  %B = zext <8 x i16> %A to <8 x i32>
  ret <8 x i32>%B
}

define <4 x i64> @zext_4i32_to_4i64(<4 x i32> %A) nounwind {
;CHECK: zext_4i32_to_4i64
;CHECK: vpunpckhdq

  %B = zext <4 x i32> %A to <4 x i64>
  ret <4 x i64>%B
}

define <4 x i32> @zext_8i8addr_to_4i32(i8* %V) nounwind {
; CHECK zext_8i8addr_to_4i32
; CHECK: vpmovzxbd
  %V1 = bitcast i8* %V to <4 x i8>*
  %V2 = load <4 x i8>* %V1, align 4
  %C = zext <4 x i8> %V2 to <4 x i32>
  ret <4 x i32>%C
}

define <4 x i32> @zext_8i8_to_4i32(<4 x i8> %A) nounwind {
; CHECK zext_8i8_to_4i32
; CHECK: vpmovzxbd
  %B = zext <4 x i8> %A to <4 x i32>
  ret <4 x i32>%B
}


define <4 x i32> @zext_4i16_to_4i32(<4 x i16> %A) nounwind {
; CHECK zext_4i16_to_4i32
; CHECK: vpmovzxwd
  %B = zext <4 x i16> %A to <4 x i32>
  ret <4 x i32>%B
}