; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.store.pd(i8 *, <8 x double>, i32, i32)

define void @f_store_pd(i8 * %arg0, <8 x double> %arg1) {
; KNF: f_store_pd:
; KNF: vstoreq   %v{{[0-9]*}}, (%{{[a-z]*}})
entry:
  call void @llvm.x86.mic.store.pd(i8 * %arg0, <8 x double> %arg1, i32 0, i32 0)

 ret void 
}

