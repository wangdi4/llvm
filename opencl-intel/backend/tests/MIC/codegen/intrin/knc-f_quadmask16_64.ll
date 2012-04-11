; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare i64 @llvm.x86.mic.quadmask16.64(i64)

define i64 @f_quadmask16_64(i64 %arg0) {
; KNF: f_quadmask16_64:
; KNF: quadmask16
entry:
  %ret = call i64 @llvm.x86.mic.quadmask16.64(i64 %arg0)

 ret i64 %ret
}

