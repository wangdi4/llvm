; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32>, i16, i8 *, i32, i32, i32)

define void @f_mask_gatherpf_ps(<16 x i32> %arg0, i16 %arg1, i8 * %arg2) {
; KNC: f_mask_gatherpf_ps:
; KNC: vgatherpf0dps
entry:
  call void @llvm.x86.mic.mask.gatherpf.ps(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 0, i32 4, i32 0)

 ret void 
}

