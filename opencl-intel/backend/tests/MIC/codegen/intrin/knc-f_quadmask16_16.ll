; XFAIL: win32

; RUN: echo
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.quadmask16.16(i16)

define i16 @f_quadmask16_16(i16 %arg0) {
; KNF: f_quadmask16_16:
; KNF: quadmask16
entry:
  %ret = call i16 @llvm.x86.mic.quadmask16.16(i16 %arg0)

 ret i16 %ret
}

