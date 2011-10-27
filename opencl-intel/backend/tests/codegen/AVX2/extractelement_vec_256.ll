; RUN: llc -mcpu=haswell < %s


define <8 x i32> @extract_insert_8_float(<8 x i32> %broadcast1, <8 x i32> %broadcast2) nounwind readnone {
; CHECK: extract_insert_8_float
; CHECK: vinserti128
  %1 = extractelement <8 x i32> %broadcast1, i32 4
  %2 = insertelement <8 x i32> %broadcast2, i32 %1, i32 6
  ret <8 x i32> %2
  }
  
define <8 x i32> @extract_insert_8_int(<8 x i32> %broadcast1, <8 x i32> %broadcast2) nounwind readnone {
; CHECK: extract_insert_8_int
; CHECK: vextracti128
; CHECK: vinserti128
  %1 = extractelement <8 x i32> %broadcast1, i32 4
  %2 = insertelement <8 x i32> %broadcast2, i32 %1, i32 6
  ret <8 x i32> %2
  }
