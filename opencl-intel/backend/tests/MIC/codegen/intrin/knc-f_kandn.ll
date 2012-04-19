; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.kandn(i16, i16)

define i16 @f_kandn(i16 %arg0, i16 %arg1) {
; KNF: f_kandn:
; KNF: vkandn
entry:
  %ret = call i16 @llvm.x86.mic.kandn(i16 %arg0, i16 %arg1)

 ret i16 %ret
}

