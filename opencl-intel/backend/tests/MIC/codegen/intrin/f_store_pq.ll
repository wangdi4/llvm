; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.store.pq(i8 *, <8 x i64>, i32, i32)

define void @f_store_pq(i8 * %arg0, <8 x i64> %arg1) {
; KNF: f_store_pq:
; KNF: vstoreq   %{{v[0-9]+}}, (%{{[a-z]+}})
entry:
  call void @llvm.x86.mic.store.pq(i8 * %arg0, <8 x i64> %arg1, i32 0, i32 0)

 ret void 
}

