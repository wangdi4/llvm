; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.rotatefield.pi(<16 x i32>, i32, i32, i32)

define <16 x i32> @f_rotatefield_pi(<16 x i32> %arg0, i32 %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_rotatefield_pi:
; KNF: vrotatefieldpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.rotatefield.pi(<16 x i32> %arg0, i32 %arg1, i32 %arg2, i32 %arg3)

 ret <16 x i32> %ret
}

