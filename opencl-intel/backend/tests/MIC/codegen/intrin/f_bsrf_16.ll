; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.bsrf.16(i16)

define i16 @f_bsrf_16(i16 %arg0) {
; KNF: f_bsrf_16:
; KNF: bsrf
entry:
  %ret = call i16 @llvm.x86.mic.bsrf.16(i16 %arg0)

 ret i16 %ret
}

