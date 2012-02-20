; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatter.ps(i8 *, <16 x i32>, <16 x float>, i32, i32, i32)

define void @f_scatter_ps(i8 * %arg0, <16 x i32> %arg1, <16 x float> %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_scatter_ps:
; KNF: vscatterps
entry:
  call void @llvm.x86.mic.scatter.ps(i8 * %arg0, <16 x i32> %arg1, <16 x float> %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret void 
}

