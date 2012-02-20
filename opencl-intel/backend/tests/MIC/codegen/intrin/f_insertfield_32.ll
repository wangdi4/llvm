; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.insertfield.32(i32, i32, i32, i32, i32)

define i32 @f_insertfield_32(i32 %arg0, i32 %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_insertfield_32:
; KNF: insertfield
entry:
  %ret = call i32 @llvm.x86.mic.insertfield.32(i32 %arg0, i32 %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret i32 %ret
}

