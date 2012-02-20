; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.store.pd(i8 *, <8 x double>, i32, i32, i32)

define void @f_store_pd(i8 * %arg0, <8 x double> %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_store_pd:
; KNF: vstorepd
entry:
  call void @llvm.x86.mic.store.pd(i8 * %arg0, <8 x double> %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret void 
}

