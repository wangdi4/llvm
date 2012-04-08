; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.packstorel.pq(i8 *, i8, <8 x i64>, i32, i32)

define void @f_mask_packstorel_pq(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, i32 %arg3, i32 %arg4) {
; KNF: f_mask_packstorel_pq:
; KNF: vpackstorelpq
entry:
  call void @llvm.x86.mic.mask.packstorel.pq(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, i32 %arg3, i32 %arg4)

 ret void 
}

