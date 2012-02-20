; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <8 x double> zeroinitializer, align 64
@pgb = common global <8 x double>* null, align 8
declare <8 x double> @llvm.x86.mic.min.pd(<8 x double>, <8 x double>)

define <8 x double> @min1(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNF: vminpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %min = call <8 x double> @llvm.x86.mic.min.pd(<8 x double> %a, <8 x double> %b)
  ret <8 x double> %min
}

define <8 x double> @min2(<8 x double> %a, <8 x double>* %p) nounwind readnone ssp {
entry:
; KNF: vminpd ({{%r[a-z0-9]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
  %b = load <8 x double>* %p
  %min = call <8 x double> @llvm.x86.mic.min.pd(<8 x double> %a, <8 x double> %b)
  ret <8 x double> %min
}

