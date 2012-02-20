; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.bsri.32(i32, i32)

define i32 @f_bsri_32(i32 %arg0, i32 %arg1) {
; KNF: f_bsri_32:
; KNF: bsri
entry:
  %ret = call i32 @llvm.x86.mic.bsri.32(i32 %arg0, i32 %arg1)

 ret i32 %ret
}

