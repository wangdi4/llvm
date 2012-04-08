; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.vprefetch1(i8 *, i32)

define void @f_vprefetch1(i8 * %arg0, i32 %arg1) {
; KNF: f_vprefetch1:
; KNF: vprefetch1
entry:
  call void @llvm.x86.mic.vprefetch1(i8 * %arg0, i32 %arg1)

 ret void 
}

