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

@gb = common global <2 x float> zeroinitializer, align 8
@pgb = common global <2 x float>* null, align 8

define double @add1(double %a.coerce, double %b.coerce) nounwind readnone ssp {
entry:
  %tmp7 = bitcast double %a.coerce to i64
  %tmp6 = bitcast i64 %tmp7 to <2 x float>
  %tmp4 = bitcast double %b.coerce to i64
  %tmp3 = bitcast i64 %tmp4 to <2 x float>
  %add = fadd <2 x float> %tmp6, %tmp3
  %tmp10 = bitcast <2 x float> %add to <1 x double>
  %tmp9 = extractelement <1 x double> %tmp10, i32 0
  ret double %tmp9
}

define double @add2(<2 x float>* nocapture %a, double %b.coerce) nounwind readonly ssp {
entry:
  %tmp4 = bitcast double %b.coerce to i64
  %tmp3 = bitcast i64 %tmp4 to <2 x float>
  %tmp1 = load <2 x float>* %a, align 8
  %add = fadd <2 x float> %tmp1, %tmp3
  %tmp7 = bitcast <2 x float> %add to <1 x double>
  %tmp6 = extractelement <1 x double> %tmp7, i32 0
  ret double %tmp6
}

define double @add3(double %a.coerce, <2 x float>* nocapture %b) nounwind readonly ssp {
entry:
  %tmp4 = bitcast double %a.coerce to i64
  %tmp3 = bitcast i64 %tmp4 to <2 x float>
  %tmp2 = load <2 x float>* %b, align 8
  %add = fadd <2 x float> %tmp3, %tmp2
  %tmp7 = bitcast <2 x float> %add to <1 x double>
  %tmp6 = extractelement <1 x double> %tmp7, i32 0
  ret double %tmp6
}

define double @add4(double %a.coerce) nounwind readonly ssp {
entry:
  %tmp3 = bitcast double %a.coerce to i64
  %tmp2 = bitcast i64 %tmp3 to <2 x float>
  %tmp1 = load <2 x float>* @gb, align 8
  %add = fadd <2 x float> %tmp2, %tmp1
  %tmp6 = bitcast <2 x float> %add to <1 x double>
  %tmp5 = extractelement <1 x double> %tmp6, i32 0
  ret double %tmp5
}

define double @add5(double %a.coerce) nounwind readonly ssp {
entry:
  %tmp4 = bitcast double %a.coerce to i64
  %tmp3 = bitcast i64 %tmp4 to <2 x float>
  %tmp1 = load <2 x float>** @pgb, align 8
  %tmp2 = load <2 x float>* %tmp1, align 8
  %add = fadd <2 x float> %tmp3, %tmp2
  %tmp7 = bitcast <2 x float> %add to <1 x double>
  %tmp6 = extractelement <1 x double> %tmp7, i32 0
  ret double %tmp6
}
