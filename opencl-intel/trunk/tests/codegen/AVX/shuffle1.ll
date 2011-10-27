; RUN: llc -mcpu=sandybridge < %s

define <4 x float> @shuf_0123(<8 x float> %broadcast2) nounwind readnone {
; CHECK: shuf_0123
; CHECK: vextractf128    $0, %ymm0
  %1 = shufflevector <8 x float> %broadcast2, <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  ret <4 x float> %1
  }

define <4 x float> @shuf_4567(<8 x float> %broadcast2) nounwind readnone {
; CHECK: shuf_4567
; CHECK: vextractf128    $1, %ymm0
; CHECK-NOT: vmov
; CHECK-NOT: vshuf
; CHECK: ret
  %1 = shufflevector <8 x float> %broadcast2, <8 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  ret <4 x float> %1
  }
