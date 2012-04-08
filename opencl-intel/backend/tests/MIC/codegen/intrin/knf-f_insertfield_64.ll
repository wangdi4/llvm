; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i64 @llvm.x86.mic.insertfield.64(i64, i64, i32, i32, i32)

define i64 @f_insertfield_64(i64 %arg0, i64 %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_insertfield_64:
; KNF: insertfield
entry:
  %ret = call i64 @llvm.x86.mic.insertfield.64(i64 %arg0, i64 %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret i64 %ret
}

