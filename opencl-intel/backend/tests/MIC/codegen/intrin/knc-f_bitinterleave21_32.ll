; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.bitinterleave21.32(i32, i32)

define i32 @f_bitinterleave21_32(i32 %arg0, i32 %arg1) {
; KNF: f_bitinterleave21_32:
; KNF: bitinterleave21
entry:
  %ret = call i32 @llvm.x86.mic.bitinterleave21.32(i32 %arg0, i32 %arg1)

 ret i32 %ret
}

