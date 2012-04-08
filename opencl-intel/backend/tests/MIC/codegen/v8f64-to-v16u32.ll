; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.cvtl.pd2pu(<16 x i32>, <8 x double>, i8);
declare <16 x i32> @llvm.x86.mic.cvth.pd2pu(<16 x i32>, <8 x double>, i8);

define <16 x i32> @cvt(<8 x double> %a, <8 x double> %b) nounwind readnone ssp {
entry:
; KNF: cvt:
; KNF: vcvtpd2pu        $0, $2, {{%v[0-9]+}}, [[R1:%v[0-9]+]]
; KNF: vcvtpd2pu        $1, $2, {{%v[0-9]+}}, [[R1]]
  %conv1 = call <16 x i32> @llvm.x86.mic.cvtl.pd2pu(<16 x i32> undef, <8 x double> %a, i8 2);
  %conv = call <16 x i32> @llvm.x86.mic.cvth.pd2pu(<16 x i32> %conv1, <8 x double> %b, i8 2);
  ret <16 x i32> %conv
}

define <16 x i32> @cvtm(<8 x double>* %pa, <8 x double>* %pb) nounwind readnone ssp {
entry:
; KNF: cvtm:
; KNF: vcvtpd2pu        $0, $2, ({{%r[a-z0-9]+}}), [[R1:%v[0-9]+]]
; KNF: vcvtpd2pu        $1, $2, ({{%r[a-z0-9]+}}), [[R1]]
  %a = load <8 x double>* %pa, align 64
  %b = load <8 x double>* %pb, align 64
  %conv1 = call <16 x i32> @llvm.x86.mic.cvtl.pd2pu(<16 x i32> undef, <8 x double> %a, i8 2);
  %conv = call <16 x i32> @llvm.x86.mic.cvth.pd2pu(<16 x i32> %conv1, <8 x double> %b, i8 2);
  ret <16 x i32> %conv
}


