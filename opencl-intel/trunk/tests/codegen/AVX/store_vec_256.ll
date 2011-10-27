; RUN: llc -mcpu=sandybridge < %s


define void @foo_8f(<8 x float>* %p, <8 x float> %x) nounwind {
; CHECK: foo_8f
; CHECK: vmovap
; CHECK: vmovap
  store <8 x float> %x, <8 x float>* %p
  ret void
}

define void @foo_4d(<4 x double>* %p, <4 x double> %x) nounwind {
; CHECK: foo_4d
; CHECK: vmovap
; CHECK: vmovap
  store <4 x double> %x, <4 x double>* %p
  ret void
}

define void @foo16_i16(<16 x i16>* %p, <16 x i16> %x) nounwind {
; CHECK: foo16_i16
; CHECK: vmovap
; CHECK: vmovap
  store <16 x i16> %x, <16 x i16>* %p
  ret void
}

define void @foo32_i8(<32 x i8>* %p, <32 x i8> %x) nounwind {
; CHECK: foo32_i8
; CHECK: vmovap
; CHECK: vmovap
  store <32 x i8> %x, <32 x i8>* %p
  ret void
}

define void @foo8_i32(<8 x i32>* %p, <8 x i32> %x) nounwind {
; CHECK: foo8_i32
; CHECK: vmovap
; CHECK: vmovap
  store <8 x i32> %x, <8 x i32>* %p
  ret void
}

define void @foo4_i64(<4 x i64>* %p, <4 x i64> %x) nounwind {
; CHECK: foo4_i64
; CHECK: vmovap
; CHECK: vmovap
  store <4 x i64> %x, <4 x i64>* %p
  ret void
}