; RUN: llc -mcpu=sandybridge < %s


define <8 x float> @extract_insert_8_float(<8 x float> %broadcast1, <8 x float> %broadcast2) nounwind readnone {
; CHECK: extract_insert_8_float
; CHECK: vinsertf128
  %1 = extractelement <8 x float> %broadcast1, i32 4
  %2 = insertelement <8 x float> %broadcast2, float %1, i32 6
  ret <8 x float> %2
  }
  
define float @extract_8_float(<8 x float> %broadcast1) nounwind readnone {
; CHECK: extract_8_float
  %1 = extractelement <8 x float> %broadcast1, i32 6
  ret float %1
  }
  
  
define <8 x i32> @extract_insert_8_int(<8 x i32> %broadcast1, <8 x i32> %broadcast2) nounwind readnone {
; CHECK: extract_insert_8_int
; CHECK: vinsertf128
  %1 = extractelement <8 x i32> %broadcast1, i32 4
  %2 = insertelement <8 x i32> %broadcast2, i32 %1, i32 6
  ret <8 x i32> %2
  }
