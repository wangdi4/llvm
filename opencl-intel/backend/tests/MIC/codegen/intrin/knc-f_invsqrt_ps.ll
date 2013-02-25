; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.invsqrt.ps(<16 x float>)

define <16 x float> @f_invsqrt_ps(<16 x float> %arg0) {
; KNF: f_invsqrt_ps:
; KNF: vrcpresps %v0, [[R0:%v[0-9]+]] 
; KNF: vmadd233ps _const_0(%{{[a-z]*}}){4to16}, [[R0]], %v{{[0-9]*}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.invsqrt.ps(<16 x float> %arg0)

 ret <16 x float> %ret
}

