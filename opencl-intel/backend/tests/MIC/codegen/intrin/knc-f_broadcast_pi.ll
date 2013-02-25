; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.broadcast.pi(i32)

define <16 x i32> @f_broadcast_pi(i32 %arg0) {
; KNF: f_broadcast_pi:
; KNF: vbroadcastpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.broadcast.pi(i32 %arg0)

 ret <16 x i32> %ret
}

