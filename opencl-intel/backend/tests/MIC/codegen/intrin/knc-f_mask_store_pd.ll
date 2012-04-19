; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.pd(i8 *, i8, <8 x double>, i32, i32)

define void @f_mask_store_pd(i8 * %arg0, i8 %arg1, <8 x double> %arg2) {
; KNF: f_mask_store_pd:
; KNF vstored   %{{v[0-9]+}}{sint16}, (%{{[a-z]*}}){nt}{%k{{[0-9]*}}}
entry:
  call void @llvm.x86.mic.mask.store.pd(i8 * %arg0, i8 %arg1, <8 x double> %arg2, i32 0, i32 1)

 ret void 
}

