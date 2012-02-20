; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.getvxcsr()

define i32 @f_getvxcsr() {
; KNF: f_getvxcsr:
; KNF: getvxcsr
entry:
  %ret = call i32 @llvm.x86.mic.getvxcsr()

 ret i32 %ret
}

