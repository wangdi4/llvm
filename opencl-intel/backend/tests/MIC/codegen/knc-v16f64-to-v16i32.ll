; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s --check-prefix=KNC

target datalayout = "e-p:64:64"

define <16 x i32> @cvt(<16 x double> %a) nounwind readnone ssp {
entry:
; KNF: cvt:
; KNF: vcvtpd2pi        $0, $2, {{%v[0-9]+}}, [[R1:%v[0-9]+]]
;
; KNC: cvt:
; KNC: vcvtfxpntpd2dq   $0, {{%zmm[0-1]}}, {{%zmm[0-9]+}}
; KNC: vcvtfxpntpd2dq   $0, {{%zmm[0-1]}}, {{%zmm[0-9]+}}
; KNC: vpermf32x4
  %conv = fptosi <16 x double> %a to <16 x i32>
  ret <16 x i32> %conv
}

define <16 x i32> @cvtm(<16 x double>* %pa) nounwind readnone ssp {
entry:
; KNF: cvtm:
; KNF: vcvtpd2pi        $0, $2, ({{%r[a-z0-9]+}}), [[R1:%v[0-9]+]]
;
; KNC: cvtm:
; KNC: vcvtfxpntpd2dq   $0, 64([[R1:%r[a-z0-9]+]]), {{%zmm[0-9]+}}
; KNC: vcvtfxpntpd2dq   $0, ([[R1]]), {{%zmm[0-9]+}}
; KNC: vpermf32x4
  %a = load <16 x double>* %pa, align 64
  %conv = fptosi <16 x double> %a to <16 x i32>
  ret <16 x i32> %conv
}

