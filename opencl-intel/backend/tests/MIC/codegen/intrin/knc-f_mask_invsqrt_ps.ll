; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.mask.invsqrt.ps(<16 x float>, i16, <16 x float>)

define <16 x float> @f_mask_invsqrt_ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2) {
; KNF: f_mask_invsqrt_ps:
; KNF: vrcpresps %v1, [[R0:%v[0-9]+]]{[[K0:%k[0-9]*]]}
; KNF: vmadd233ps _const_0(%{{[a-z]*}}){4to16}, [[R0]], %v{{[0-9]*}}{[[K0]]}
entry:
  %ret = call <16 x float> @llvm.x86.mic.mask.invsqrt.ps(<16 x float> %arg0, i16 %arg1, <16 x float> %arg2)

 ret <16 x float> %ret
}

