; RUN: llc -mcpu=corei7-avx < %s


define void @foo_8f(ptr %p, <8 x float> %x) nounwind {
; CHECK: foo_8f
; CHECK: vmovap
; CHECK: vmovap
  store <8 x float> %x, ptr %p
  ret void
}

define void @foo_4d(ptr %p, <4 x double> %x) nounwind {
; CHECK: foo_4d
; CHECK: vmovap
; CHECK: vmovap
  store <4 x double> %x, ptr %p
  ret void
}

define void @foo16_i16(ptr %p, <16 x i16> %x) nounwind {
; CHECK: foo16_i16
; CHECK: vmovap
; CHECK: vmovap
  store <16 x i16> %x, ptr %p
  ret void
}

define void @foo32_i8(ptr %p, <32 x i8> %x) nounwind {
; CHECK: foo32_i8
; CHECK: vmovap
; CHECK: vmovap
  store <32 x i8> %x, ptr %p
  ret void
}

define void @foo8_i32(ptr %p, <8 x i32> %x) nounwind {
; CHECK: foo8_i32
; CHECK: vmovap
; CHECK: vmovap
  store <8 x i32> %x, ptr %p
  ret void
}

define void @foo4_i64(ptr %p, <4 x i64> %x) nounwind {
; CHECK: foo4_i64
; CHECK: vmovap
; CHECK: vmovap
  store <4 x i64> %x, ptr %p
  ret void
}