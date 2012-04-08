; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare double @llvm.x86.mic.mask.reduce.gmin.pd(i8, <8 x double>)

define double @f_mask_reduce_gmin_pd(i8 %arg0, <8 x double> %arg1) {
; KNF: f_mask_reduce_gmin_pd:
; KNF: vreducepd
entry:
  %ret = call double @llvm.x86.mic.mask.reduce.gmin.pd(i8 %arg0, <8 x double> %arg1)

 ret double %ret
}

