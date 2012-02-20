; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %s -check-prefix=KNFmpa 
;

target datalayout = "e-p:64:64"

@gb = common global <8 x double> zeroinitializer, align 64
@pgb = common global <8 x double>* null, align 8

define <8 x double> @madd1_cmbpa(<8 x double> %a, <8 x double> %b, <8 x double> %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[123]+pd}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul <8 x double> %c, %b
  %add = fadd <8 x double> %a, %mul
  ret <8 x double> %add
}

define <8 x double> @madd2_Mambpc(<8 x double>* nocapture %a, <8 x double> %b, <8 x double> %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+pd}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <8 x double>* %a, align 64
  %mul = fmul <8 x double> %b, %tmp1
  %add = fadd <8 x double> %mul, %c
  ret <8 x double> %add
}

define <8 x double> @madd3_Mbmapc(<8 x double> %a, <8 x double>* nocapture %b, <8 x double> %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+pd}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp2 = load <8 x double>* %b, align 64
  %mul = fmul <8 x double> %tmp2, %a
  %add = fadd <8 x double> %mul, %c
  ret <8 x double> %add
}

define <8 x double> @madd3_bmapMc(<8 x double> %a, <8 x double> %b, <8 x double>* nocapture %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+pd}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul <8 x double> %b, %a
  %tmp3 = load <8 x double>* %c, align 64
  %add = fadd <8 x double> %tmp3, %mul
  ret <8 x double> %add
}

define <8 x double> @madd4_Mgmapb(<8 x double> %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: {{vmadd[123]+pd}} ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNFmpa: vmulpd ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <8 x double>* @gb, align 64
  %mul = fmul <8 x double> %tmp1, %a
  %add = fadd <8 x double> %mul, %b
  ret <8 x double> %add
}

define <8 x double> @madd5_bmapMMg(<8 x double> %a, <8 x double> %b) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: {{vmadd[123]+pd}} ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulpd {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNFmpa: movq ([[R1]]), [[R2:%[a-z]+]]
; KNFmpa: vaddpd ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <8 x double>** @pgb, align 8
  %tmp2 = load <8 x double>* %tmp1, align 64
  %mul = fmul <8 x double> %b, %a
  %add = fadd <8 x double> %tmp2, %mul
  ret <8 x double> %add
}
