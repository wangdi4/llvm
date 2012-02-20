; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.mask2int(i16)

define i32 @f_mask2int(i16 %arg0) {
; KNF: f_mask2int:
; KNF: mask2int
entry:
  %ret = call i32 @llvm.x86.mic.mask2int(i16 %arg0)

 ret i32 %ret
}

