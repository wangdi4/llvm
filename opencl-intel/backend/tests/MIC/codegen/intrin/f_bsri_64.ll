; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i64 @llvm.x86.mic.bsri.64(i64, i64)

define i64 @f_bsri_64(i64 %arg0, i64 %arg1) {
; KNF: f_bsri_64:
; KNF: bsri
entry:
  %ret = call i64 @llvm.x86.mic.bsri.64(i64 %arg0, i64 %arg1)

 ret i64 %ret
}

