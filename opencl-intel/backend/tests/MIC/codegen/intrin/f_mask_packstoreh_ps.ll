; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.packstoreh.ps(i8 *, i16, <16 x float>, i32, i32)

define void @f_mask_packstoreh_ps(i8 * %arg0, i16 %arg1, <16 x float> %arg2) {
; KNF: f_mask_packstoreh_ps:
; KNF: vkmov     %{{[a-z]*}}, %k{{[0-9]*}}
; KNF: vpackstorehd %v{{[0-9]*}}{uint8}, (%{{[a-z]*}}){%k{{[0-9]*}}} 
entry:
  call void @llvm.x86.mic.mask.packstoreh.ps(i8 * %arg0, i16 %arg1, <16 x float> %arg2, i32 2, i32 0)

 ret void 
}

