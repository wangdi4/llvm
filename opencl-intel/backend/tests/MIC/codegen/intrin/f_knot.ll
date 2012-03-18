; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.knot(i16)

define i16 @f_knot(i16 %arg0) {
; KNF: f_knot:
; KNF: vknot %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  %ret = call i16 @llvm.x86.mic.knot(i16 %arg0)

 ret i16 %ret
}

