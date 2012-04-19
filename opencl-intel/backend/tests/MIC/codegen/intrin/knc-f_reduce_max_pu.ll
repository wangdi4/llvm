; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.reduce.max.pu(<16 x i32>)

define i32 @f_reduce_max_pu(<16 x i32> %arg0) {
; KNF: f_reduce_max_pu:
; KNF: vreducepu
entry:
  %ret = call i32 @llvm.x86.mic.reduce.max.pu(<16 x i32> %arg0)

 ret i32 %ret
}

