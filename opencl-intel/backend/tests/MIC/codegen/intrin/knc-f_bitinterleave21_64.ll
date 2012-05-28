; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i64 @llvm.x86.mic.bitinterleave21.64(i64, i64)

define i64 @f_bitinterleave21_64(i64 %arg0, i64 %arg1) {
; KNF: f_bitinterleave21_64:
; KNF: bitinterleave21
entry:
  %ret = call i64 @llvm.x86.mic.bitinterleave21.64(i64 %arg0, i64 %arg1)

 ret i64 %ret
}

