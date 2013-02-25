; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.gather.ps(<16 x i32>, i8 *, i32, i32, i32)

define <16 x float> @f_gather_ps(<16 x i32> %arg0, i8 * %arg1) {
; KNF: f_gather_ps:
; KNF: vkxnor %k{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vgatherd (%{{[a-z]*}},%v{{[0-9]*}},4), %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vorpi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.gather.ps(<16 x i32> %arg0, i8 * %arg1, i32 0, i32 4, i32 0)

 ret <16 x float> %ret
}

