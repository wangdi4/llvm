; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.bsri.16(i16, i16)

define i16 @f_bsri_16(i16 %arg0, i16 %arg1) {
; KNF: f_bsri_16:
; KNF: bsri
entry:
  %ret = call i16 @llvm.x86.mic.bsri.16(i16 %arg0, i16 %arg1)

 ret i16 %ret
}

