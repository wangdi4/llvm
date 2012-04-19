; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.countbits.32(i32)

define i32 @f_countbits_32(i32 %arg0) {
; KNF: f_countbits_32:
; KNF: countbits
entry:
  %ret = call i32 @llvm.x86.mic.countbits.32(i32 %arg0)

 ret i32 %ret
}

