; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.packstorel.pd(i8 *, i8, <8 x double>, i32, i32)

define void @f_mask_packstorel_pd(i8 * %arg0, i8 %arg1, <8 x double> %arg2) {
; KNF: f_mask_packstorel_pd:
; KNF: vkmov     %{{[a-z]*}}, %k{{[0-9]*}}
; KNF: vpackstorelq %v{{[0-9]*}}, (%{{[a-z]*}}){%k{{[0-9]*}}}

entry:
  call void @llvm.x86.mic.mask.packstorel.pd(i8 * %arg0, i8 %arg1, <8 x double> %arg2, i32 0, i32 0)

 ret void 
}

