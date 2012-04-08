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

@gb = common global <16 x float> zeroinitializer, align 64
@pgb = common global <16 x float>* null, align 8

define <16 x float> @madd1_cmbpa(<16 x float> %a, <16 x float> %b, <16 x float> %c) nounwind readnone ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul <16 x float> %c, %b
  %add = fadd <16 x float> %a, %mul
  ret <16 x float> %add
}

define <16 x float> @madd2_Mambpc(<16 x float>* nocapture %a, <16 x float> %b, <16 x float> %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <16 x float>* %a, align 64
  %mul = fmul <16 x float> %b, %tmp1
  %add = fadd <16 x float> %mul, %c
  ret <16 x float> %add
}

define <16 x float> @madd3_Mbmapc(<16 x float> %a, <16 x float>* nocapture %b, <16 x float> %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp2 = load <16 x float>* %b, align 64
  %mul = fmul <16 x float> %tmp2, %a
  %add = fadd <16 x float> %mul, %c
  ret <16 x float> %add
}

define <16 x float> @madd3_bmapMc(<16 x float> %a, <16 x float> %b, <16 x float>* nocapture %c) nounwind readonly ssp {
entry:
; KNF: {{vmadd[123]+ps}} {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{\(%[a-z]+\)}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %mul = fmul <16 x float> %b, %a
  %tmp3 = load <16 x float>* %c, align 64
  %add = fadd <16 x float> %tmp3, %mul
  ret <16 x float> %add
}

define <16 x float> @madd4_Mgmapb(<16 x float> %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: {{vmadd[123]+ps}} ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNFmpa: vmulps ([[R1]]), {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vaddps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <16 x float>* @gb, align 64
  %mul = fmul <16 x float> %tmp1, %a
  %add = fadd <16 x float> %mul, %b
  ret <16 x float> %add
}

define <16 x float> @madd5_bmapMMg(<16 x float> %a, <16 x float> %b) nounwind readonly ssp {
entry:
; KNF: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNF: movq ([[R1]]), [[R2:%[a-z]+]]
; KNF: {{vmadd[123]+ps}} ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNFmpa: movq ([[R1]]), [[R2:%[a-z]+]]
; KNFmpa: vaddps ([[R2]]), {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load <16 x float>** @pgb, align 8
  %tmp2 = load <16 x float>* %tmp1, align 64
  %mul = fmul <16 x float> %b, %a
  %add = fadd <16 x float> %tmp2, %mul
  ret <16 x float> %add
}
