; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.kortestc(i16, i16)

define i32 @f_kortestc(i16 %arg0, i16 %arg1) {
; KNF: f_kortestc:
; KNF: vkortest
; KNF: setb
; KNF: ret
entry:
  %ret = call i32 @llvm.x86.mic.kortestc(i16 %arg0, i16 %arg1)

 ret i32 %ret
}

