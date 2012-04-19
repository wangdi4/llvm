; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.int2mask(i32)

define i16 @f_int2mask(i32 %arg0) {
; KNF: f_int2mask:
; KNF: int2mask
entry:
  %ret = call i16 @llvm.x86.mic.int2mask(i32 %arg0)

 ret i16 %ret
}

