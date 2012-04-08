; XFAIL: win32
; XFAIL: *
; 
; REVIEW: Once this compiles, we need to add KNF: lines with the
;         expected output.
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <3 x float> zeroinitializer, align 16
@pgb = common global <3 x float>* null, align 8

define <2 x double> @add1(<2 x double> %a.coerce, <2 x double> %b.coerce) nounwind readnone ssp {
entry:
  %tmp9 = bitcast <2 x double> %a.coerce to i128
  %tmp7 = trunc i128 %tmp9 to i96
  %tmp8 = bitcast i96 %tmp7 to <3 x float>
  %tmp5 = bitcast <2 x double> %b.coerce to i128
  %tmp3 = trunc i128 %tmp5 to i96
  %tmp4 = bitcast i96 %tmp3 to <3 x float>
  %add = fadd <3 x float> %tmp8, %tmp4
  %tmp12 = bitcast <3 x float> %add to i96
  %tmp13 = zext i96 %tmp12 to i128
  %tmp11 = bitcast i128 %tmp13 to <2 x double>
  ret <2 x double> %tmp11
}

define <2 x double> @add2(<3 x float>* nocapture %a, <2 x double> %b.coerce) nounwind readonly ssp {
entry:
  %tmp5 = bitcast <2 x double> %b.coerce to i128
  %tmp3 = trunc i128 %tmp5 to i96
  %tmp4 = bitcast i96 %tmp3 to <3 x float>
  %tmp1 = load <3 x float>* %a, align 16
  %add = fadd <3 x float> %tmp1, %tmp4
  %tmp8 = bitcast <3 x float> %add to i96
  %tmp9 = zext i96 %tmp8 to i128
  %tmp7 = bitcast i128 %tmp9 to <2 x double>
  ret <2 x double> %tmp7
}

define <2 x double> @add3(<2 x double> %a.coerce, <3 x float>* nocapture %b) nounwind readonly ssp {
entry:
  %tmp5 = bitcast <2 x double> %a.coerce to i128
  %tmp3 = trunc i128 %tmp5 to i96
  %tmp4 = bitcast i96 %tmp3 to <3 x float>
  %tmp2 = load <3 x float>* %b, align 16
  %add = fadd <3 x float> %tmp4, %tmp2
  %tmp8 = bitcast <3 x float> %add to i96
  %tmp9 = zext i96 %tmp8 to i128
  %tmp7 = bitcast i128 %tmp9 to <2 x double>
  ret <2 x double> %tmp7
}

define <2 x double> @add4(<2 x double> %a.coerce) nounwind readonly ssp {
entry:
  %tmp4 = bitcast <2 x double> %a.coerce to i128
  %tmp2 = trunc i128 %tmp4 to i96
  %tmp3 = bitcast i96 %tmp2 to <3 x float>
  %tmp1 = load <3 x float>* @gb, align 16
  %add = fadd <3 x float> %tmp3, %tmp1
  %tmp7 = bitcast <3 x float> %add to i96
  %tmp8 = zext i96 %tmp7 to i128
  %tmp6 = bitcast i128 %tmp8 to <2 x double>
  ret <2 x double> %tmp6
}

define <2 x double> @add5(<2 x double> %a.coerce) nounwind readonly ssp {
entry:
  %tmp5 = bitcast <2 x double> %a.coerce to i128
  %tmp3 = trunc i128 %tmp5 to i96
  %tmp4 = bitcast i96 %tmp3 to <3 x float>
  %tmp1 = load <3 x float>** @pgb, align 8
  %tmp2 = load <3 x float>* %tmp1, align 16
  %add = fadd <3 x float> %tmp4, %tmp2
  %tmp8 = bitcast <3 x float> %add to i96
  %tmp9 = zext i96 %tmp8 to i128
  %tmp7 = bitcast i128 %tmp9 to <2 x double>
  ret <2 x double> %tmp7
}
