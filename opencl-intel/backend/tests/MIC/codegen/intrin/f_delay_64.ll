; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.delay.64(i64)

define void @f_delay_64(i64 %arg0) {
; KNF: f_delay_64:
; KNF: delay
entry:
  call void @llvm.x86.mic.delay.64(i64 %arg0)

 ret void 
}

