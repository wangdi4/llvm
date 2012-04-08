; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.packstoreh.pi(i8 *, <16 x i32>, i32, i32)

define void @f_packstoreh_pi(i8 * %arg0, <16 x i32> %arg1, i32 %arg2, i32 %arg3) {
; KNF: f_packstoreh_pi:
; KNF: vpackstorehpi
entry:
  call void @llvm.x86.mic.packstoreh.pi(i8 * %arg0, <16 x i32> %arg1, i32 %arg2, i32 %arg3)

 ret void 
}

