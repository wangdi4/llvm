; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.kmov(i16)

define i16 @f_kmov(i16 %arg0) {
; KNF: f_kmov:
; KNF: movzwl    %{{[a-z]*}}, %{{[a-z]*}}
entry:
  %ret = call i16 @llvm.x86.mic.kmov(i16 %arg0)

 ret i16 %ret
}

