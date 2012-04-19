; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.pq(i8 *, i8, <8 x i64>, i32, i32)

define void @f_mask_store_pq(i8 * %arg0, i8 %arg1, <8 x i64> %arg2) {
; KNF: f_mask_store_pq:
; KNF: vstoreq   %{{v[0-9]+}}, (%{{[a-z]+}})
entry:
  call void @llvm.x86.mic.mask.store.pq(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, i32 0, i32 0)

 ret void 
}

