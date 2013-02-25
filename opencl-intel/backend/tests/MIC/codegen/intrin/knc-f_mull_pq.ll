; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.mull.pq(<8 x i64>, <8 x i64>)

define <8 x i64> @f_mull_pq(<8 x i64> %arg0, <8 x i64> %arg1) {
; KNF: f_mull_pq:
; KNF: vmadd231pi %v{{[0-9]*}}{cdab}, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vmadd231pi %v{{[0-9]*}}{cdab}, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vshuf128x32 $160, $228, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vorpi     %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <8 x i64> @llvm.x86.mic.mull.pq(<8 x i64> %arg0, <8 x i64> %arg1)

 ret <8 x i64> %ret
}

