; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.packstorel.ps(i8 *, <16 x float>, i32, i32)

define void @f_packstorel_ps(i8 * %arg0, <16 x float> %arg1) {
; KNF: f_packstorel_ps:
; KNF: vpackstoreld %v{{[0-9]*}}{uint8}, (%{{[a-z]*}})
entry:
  call void @llvm.x86.mic.packstorel.ps(i8 * %arg0, <16 x float> %arg1, i32 2, i32 0)

 ret void 
}

