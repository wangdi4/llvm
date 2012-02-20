; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; REVIEW: Both tests should generate FMA. Using FAM here is required for 
; correctness
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %s -check-prefix=KNFmpa 
;
; REVIEW: Disabled KNC for now because we don't have divide.
; NUR: llc < %s -mtriple=x86_64-pc-linux \
; NUR:       -march=y86-64 -mcpu=knc \
; NUR:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@g = common global <16 x float> zeroinitializer, align 64

define <16 x float> @add1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vaddps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vaddps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %add = fadd <16 x float> %tmp1, %a
  ret <16 x float> %add
}

define <16 x float> @add2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vaddps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vaddps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %add = fadd <16 x float> %a, %tmp
  ret <16 x float> %add
}

define <16 x float> @mul1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vmulps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vmulps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %mul = fmul <16 x float> %tmp1, %a
  ret <16 x float> %mul
}

define <16 x float> @mul2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vmulps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vmulps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %mul = fmul <16 x float> %a, %tmp
  ret <16 x float> %mul
}

define <16 x float> @sub1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vsubps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vsubps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %sub = fsub <16 x float> %a, %tmp1
  ret <16 x float> %sub
}

define <16 x float> @sub2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vsubrps  {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+\(%[a-z]+\)}}, [[R1:%[a-z]+]]
; KNC: vsubrps ([[R1]]), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %sub = fsub <16 x float> %tmp, %a
  ret <16 x float> %sub
}

define <16 x float> @div1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vrcpresps {{[^(]+}}(%rip), [[R1:%v[0-9]+]]
; KNF: vrcprefineps {{[^(]+}}(%rip), [[R1]], [[R2:%v[0-9]+]]
; KNF: vmsubr23c1ps {{[^(]+}}(%rip), [[R2]], [[R3:%v[0-9]+]]
; KNF: vmadd132ps  [[R3]], [[R2]], [[R4:%v[0-9]+]]
; KNF: vmsubr23c1ps {{[^(]+}}(%rip), [[R2]], [[R4:%v[0-9]+]] 
; KNF: vmadd132ps [[R4]], [[R2]], [[R2]]
; KNF: vfixupps $100418, {{[^(]+}}(%rip), [[R2]] 
;
; KNFmpa: vrcpresps {{[^(]+}}(%rip), [[R1:%v[0-9]+]]
; KNFmpa: vrcprefineps {{[^(]+}}(%rip), [[R1]], [[R2:%v[0-9]+]]
; KNFmpa: vmsubr23c1ps {{[^(]+}}(%rip), [[R2]], [[R3:%v[0-9]+]]
; KNFmpa: vmadd132ps  [[R3]], [[R2]], [[R4:%v[0-9]+]]
; KNFmpa: vmsubr23c1ps {{[^(]+}}(%rip), [[R2]], [[R4:%v[0-9]+]] 
; KNFmpa: vmadd132ps [[R4]], [[R2]], [[R2]]
; KNFmpa: vfixupps $100418, {{[^(]+}}(%rip), [[R2]] 
;
  %tmp1 = load <16 x float>* @g, align 64
  %div = fdiv <16 x float> %a, %tmp1
  ret <16 x float> %div
}

define <16 x float> @div2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNF: vrcpresps [[R1:%v[0-9]+]], [[R2:%v[0-9]+]]                          
; KNF: vrcprefineps [[R1]], [[R2]], [[R3:%v[0-9]+]]			    
; KNF: vmsubr23c1ps [[R1]], [[R3]], [[R4:%v[0-9]+]]
; KNF: vmadd132ps  [[R4]], [[R3]], [[R3]]
; KNF: vmsubr23c1ps [[R1]], [[R3]], [[R5:%v[0-9]+]]			 
; KNF: vmadd132ps [[R1]], [[R6:%v[0-9]+]], [[R3]]			 
; KNF: vfixupps $132737, [[R6]], [[R3]]
;
; KNFmpa: vrcpresps [[R1:%v[0-9]+]], [[R2:%v[0-9]+]]                          
; KNFmpa: vrcprefineps [[R1]], [[R2]], [[R3:%v[0-9]+]]			    
; KNFmpa: vmsubr23c1ps [[R1]], [[R3]], [[R4:%v[0-9]+]]
; KNFmpa: vmadd132ps  [[R4]], [[R3]], [[R3]]
; KNFmpa: vmsubr23c1ps [[R1]], [[R3]], [[R5:%v[0-9]+]]			 
; KNFmpa: vmadd132ps [[R1]], [[R6:%v[0-9]+]], [[R3]]			 
; KNFmpa: vfixupps $132737, [[R6]], [[R3]]
;
  %tmp = load <16 x float>* @g, align 64
  %div = fdiv <16 x float> %tmp, %a
  ret <16 x float> %div
}
