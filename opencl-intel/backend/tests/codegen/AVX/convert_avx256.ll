; RUN: llc -mcpu=sandybridge < %s

define <4 x double> @convert_float4_to_double4(<4 x float> %x) nounwind readnone {
entry1:
; CHECK: convert_float4_to_double4
; CHECK: vcvtps2pd
  %0 = fpext <4 x float> %x to <4 x double>                
  ret <4 x double> %0
}

define <4 x float> @convert_double4_to_float4(<4 x double> %X) nounwind readnone {
entry:
; CHECK: convert_double4_to_float4
; CHECK: vcvtpd2ps
  %0 = fptrunc <4 x double> %X to <4 x float>                
  ret <4 x float> %0
}


define <8 x float> @convert_int_to_float(<8 x i32> %X) nounwind readnone {
entry:
; CHECK: convert_int_to_float
; CHECK: vcvtdq2ps
  %0 = sitofp <8 x i32> %X to <8 x float>                    
  ret <8 x float> %0
}

define <4 x double> @convert_int_to_double(<4 x i32> %X) nounwind readnone {
entry:
; CHECK: convert_int_to_double
; CHECK: vcvtdq2pd
  %0 = sitofp <4 x i32> %X to <4 x double>                    
  ret <4 x double> %0
}

define <8 x i32> @convert_float_to_int8(<8 x float> %X) nounwind readnone {
entry:
; CHECK: convert_float_to_int8
; CHECK: vcvttps2dq
  %0 = fptosi <8 x float> %X to <8 x i32>                   
  ret <8 x i32> %0
}

define <4 x i32> @convert_float_to_int4(<4 x float> %X) nounwind readnone {
entry:
; CHECK: convert_float_to_int4
; CHECK: vcvttps2dq
  %0 = fptosi <4 x float> %X to <4 x i32>                   
  ret <4 x i32> %0
}
