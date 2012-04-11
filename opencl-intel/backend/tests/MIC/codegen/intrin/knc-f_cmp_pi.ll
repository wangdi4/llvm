; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.cmp.pi(<16 x i32>, <16 x i32>, i32)

define i16 @f_cmp_pi(<16 x i32> %arg0, <16 x i32> %arg1, i32 %arg2) {
; KNF: f_cmp_pi:
; KNF: vcmppi
entry:
  %ret = call i16 @llvm.x86.mic.cmp.pi(<16 x i32> %arg0, <16 x i32> %arg1, i32 %arg2)

 ret i16 %ret
}

