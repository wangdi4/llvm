; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatterpf.ps(i8 *, <16 x i32>, i32, i32, i32)

define void @f_scatterpf_ps(i8 * %arg0, <16 x i32> %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_scatterpf_ps:
; KNF: vscatterpfps
entry:
  call void @llvm.x86.mic.scatterpf.ps(i8 * %arg0, <16 x i32> %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret void 
}

