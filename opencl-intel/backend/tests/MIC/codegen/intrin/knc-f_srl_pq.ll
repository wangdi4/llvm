; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.srl.pq(<8 x i64>, <8 x i64>)

define <8 x i64> @f_srl_pq(<8 x i64> %arg0, <8 x i64> %arg1) {
; KNF: f_srl_pq:
; KNF: vsllpi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vsrlpi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vsrlpi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <8 x i64> @llvm.x86.mic.srl.pq(<8 x i64> %arg0, <8 x i64> %arg1)

 ret <8 x i64> %ret
}

