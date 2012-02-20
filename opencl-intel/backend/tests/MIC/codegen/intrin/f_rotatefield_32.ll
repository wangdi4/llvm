; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.rotatefield.32(i32, i32, i32, i32)

define i32 @f_rotatefield_32(i32 %arg0, i32 %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_rotatefield_32:
; KNF: rotatefield
entry:
  %ret = call i32 @llvm.x86.mic.rotatefield.32(i32 %arg0, i32 %arg1, i32 %arg2, i32 %arg3)

 ret i32 %ret
}

