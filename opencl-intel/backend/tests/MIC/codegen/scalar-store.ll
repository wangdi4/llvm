; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define void @addf(float* nocapture %p, float %a, float %b) nounwind ssp {
entry:
  %add = fadd float %a, %b
; KNF: vstored %{{v[0-9]+}}{a}, {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: movl $1, %e[[R1:[a-z]+]]
; KNC: kmovd %r[[R1]], [[R2:%k[0-9]+]]
; KNC: vpackstoreld {{%zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}}){[[R2]]}
  store float %add, float* %p
  ret void
}

define void @addd(double* nocapture %p, double %a, double %b) nounwind ssp {
entry:
  %add = fadd double %a, %b
; KNF: vstoreq %{{v[0-9]+}}{a}, {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: movl $1, %e[[R1:[a-z]+]]
; KNC: kmovd %r[[R1]], [[R2:%k[0-9]+]]
; KNC: vpackstorelq {{%zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}}){[[R2]]}
  store double %add, double* %p
  ret void
}

define void @addint16(i32* nocapture %p, <16 x i32>* nocapture %pf, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
  %add = add nsw <16 x i32> %a, %b
; KNF: vstored %{{v[0-9]+}}, -{{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: vmovdqa32 %{{zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}})
  store <16 x i32> %add, <16 x i32>* %pf
  %tmp5 = extractelement <16 x i32> %add, i32 0
; KNF: vstored %{{v[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}})
; KNF: movl -{{([0-9]+)?}}(%{{[a-z]+}}), %{{[a-z]+}}
; KNF: movl %{{[a-z]+}}, (%{{[a-z]+}})
;
; KNC: vmovdqa32 %{{zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}})
; KNC: movl (%{{[a-z]+}}), %{{[a-z]+}}
; KNC: movl %{{[a-z]+}}, (%{{[a-z]+}})
  store i32 %tmp5, i32* %p
  ret void
}

define void @addfloat16(float* nocapture %p, <16 x float>* nocapture %pf, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
  %add = fadd <16 x float> %a, %b
; KNF: vstored [[R1:%v[0-9]+]], {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: vmovaps %{{zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}})
  store <16 x float> %add, <16 x float>* %pf
  %tmp5 = extractelement <16 x float> %add, i32 0
; KNF: vstored [[R1]]{a}, {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: vmovaps %{{zmm[0-9]+}}, [[M1:([0-9]+)?\(%[a-z]+\)]]
; KNC: movl $1, %e[[R1:[a-z]+]]
; KNC: kmovd %r[[R1]], [[R2:%k[0-9]+]]
; KNC: vbroadcastss [[M1]], {{%zmm[0-9]+}}
; KNC: vpackstoreld {{%zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}}){[[R2]]}
  store float %tmp5, float* %p
  ret void
}

define void @adddouble8(double* nocapture %p, <8 x double>* nocapture %pf, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
  %add = fadd <8 x double> %a, %b
; KNF: vstoreq [[R1:%v[0-9]+]], {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: vmovapd %{{zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}})
  store <8 x double> %add, <8 x double>* %pf
  %tmp5 = extractelement <8 x double> %add, i32 0
; KNF: vstoreq [[R1]]{a}, {{([0-9]+)?}}(%{{[a-z]+}})
;
; KNC: vmovapd %{{zmm[0-9]+}}, [[M1:([0-9]+)?\(%[a-z]+\)]]
; KNC: movl $1, %e[[R1:[a-z]+]]
; KNC: kmovd %r[[R1]], [[R2:%k[0-9]+]]
; KNC: vbroadcastsd [[M1]], {{%zmm[0-9]+}}
; KNC: vpackstorelq {{%zmm[0-9]+}}, {{([0-9]+)?}}(%{{[a-z]+}}){[[R2]]}
  store double %tmp5, double* %p
  ret void
}
