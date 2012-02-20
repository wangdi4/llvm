; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.test.pi(<16 x i32>, <16 x i32>)

define i16 @f_test_pi(<16 x i32> %arg0, <16 x i32> %arg1) {
; KNF: f_test_pi:
; KNF: vtestpi
entry:
  %ret = call i16 @llvm.x86.mic.test.pi(<16 x i32> %arg0, <16 x i32> %arg1)

 ret i16 %ret
}

