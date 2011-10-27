; RUN: llc -mcpu=haswell < %s

define <4 x i64> @insertelement_4_i64(<4 x i64> %broadcast2) nounwind readnone {
; CHECK: insertelement_4_i64
; CHECK: vinserti128
  %1 = add <4 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3>
  %2 = insertelement <4 x i64> %1, i64 8, i32 3
  ret <4 x i64> %2
  }

define <8 x i32> @insertelement_8_i32(<8 x i32> %broadcast2) nounwind readnone {
; CHECK: insertelement_8_i32
; CHECK: vinserti128
  %1 = add <8 x i32> %broadcast2, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  %2 = insertelement <8 x i32> %1, i32 8, i32 5
  ret <8 x i32> %2
  }
