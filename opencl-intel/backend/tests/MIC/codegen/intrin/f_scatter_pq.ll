; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatter.pq(i8 *, <8 x i64>, <8 x i64>, i32, i32, i32)

define void @f_scatter_pq(i8 * %arg0, <8 x i64> %arg1, <8 x i64> %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_scatter_pq:
; KNF: vscatterpq
entry:
  call void @llvm.x86.mic.scatter.pq(i8 * %arg0, <8 x i64> %arg1, <8 x i64> %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret void 
}

