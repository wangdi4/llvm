; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.ps(i8 *, i16, <16 x float>, i32, i32, i32)

define void @f_mask_store_ps(i8 * %arg0, i16 %arg1, <16 x float> %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_mask_store_ps:
; KNF: vstoreps
entry:
  call void @llvm.x86.mic.mask.store.ps(i8 * %arg0, i16 %arg1, <16 x float> %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret void 
}

