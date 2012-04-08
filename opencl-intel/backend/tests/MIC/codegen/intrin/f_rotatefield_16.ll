; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.rotatefield.16(i16, i32, i32, i32)

define i16 @f_rotatefield_16(i16 %arg0, i32 %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_rotatefield_16:
; KNF: rotatefield
entry:
  %ret = call i16 @llvm.x86.mic.rotatefield.16(i16 %arg0, i32 %arg1, i32 %arg2, i32 %arg3)

 ret i16 %ret
}

