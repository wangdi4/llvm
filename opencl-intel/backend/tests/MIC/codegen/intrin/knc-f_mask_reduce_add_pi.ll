; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.mask.reduce.add.pi(i16, <16 x i32>)

define i32 @f_mask_reduce_add_pi(i16 %arg0, <16 x i32> %arg1) {
; KNF: f_mask_reduce_add_pi:
; KNF: vreducepi
entry:
  %ret = call i32 @llvm.x86.mic.mask.reduce.add.pi(i16 %arg0, <16 x i32> %arg1)

 ret i32 %ret
}

