; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.store.pi(i8 *, <16 x i32>, i32, i32)

define void @f_store_pi(i8 * %arg0, <16 x i32> %arg1) {
; KNF: f_store_pi:
; KNF: vstored   %{{v[0-9]+}}, (%{{[a-z]+}})
entry:
  call void @llvm.x86.mic.store.pi(i8 * %arg0, <16 x i32> %arg1, i32 0, i32 0)

 ret void 
}

