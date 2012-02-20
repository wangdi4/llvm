; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.cmple.pu(<16 x i32>, <16 x i32>)

define i16 @f_cmple_pu(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNF: f_cmple_pu:
; KNF: vcmppu
entry:
  %ret = call i16 @llvm.x86.mic.cmple.pu(<16 x i32> %arg0, <16 x i32> %arg1)

 ret i16 %ret
}

