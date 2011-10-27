; RUN: llc -mcpu=sandybridge < %s

define <4 x double> @insertelement_4_double_3(<4 x double> %broadcast2) nounwind readnone {
; CHECK: insertelement_4_double_3
; CHECK: vinsertf128
  %1 = insertelement <4 x double> %broadcast2, double 8.1, i32 3
  ret <4 x double> %1
  }

define <4 x double> @insertelement_4_double_0(<4 x double> %broadcast2) nounwind readnone {
; CHECK: insertelement_4_double_0
; CHECK: vinsertf128
  %1 = insertelement <4 x double> %broadcast2, double 8.1, i32 0
  ret <4 x double> %1
  }
