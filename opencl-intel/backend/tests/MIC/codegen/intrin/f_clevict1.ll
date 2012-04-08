; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.clevict1(i8 *)

define void @f_clevict1(i8 * %arg0) {
; KNF: f_clevict1:
; KNF: clevict1
entry:
  call void @llvm.x86.mic.clevict1(i8 * %arg0)

 ret void 
}

