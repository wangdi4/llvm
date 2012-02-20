; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.gatherpf.ps(<16 x i32>, i8 *, i32, i32, i32)

define void @f_gatherpf_ps(<16 x i32> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_gatherpf_ps:
; KNF: vgatherpfps
entry:
  call void @llvm.x86.mic.gatherpf.ps(<16 x i32> %arg0, i8 * %arg1, i32 %arg2, i32 %arg3, i32 %arg4)

 ret void 
}

