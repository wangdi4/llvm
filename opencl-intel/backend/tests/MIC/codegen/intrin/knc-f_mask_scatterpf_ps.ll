; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.scatterpf.ps(i8 *, i16, <16 x i32>, i32, i32, i32)

define void @f_mask_scatterpf_ps(i8 * %arg0, i16 %arg1, <16 x i32> %arg2) {
; KNC: f_mask_scatterpf_ps:
; KNC: vscatterpf0dps
entry:
  call void @llvm.x86.mic.mask.scatterpf.ps(i8 * %arg0, i16 %arg1, <16 x i32> %arg2, i32 0, i32 4, i32 0)

 ret void 
}

