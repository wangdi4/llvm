; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.store.ps(i8 *, <16 x float>, i32, i32, i32)

define void @f_store_ps(i8 * %arg0, <16 x float> %arg1) {
; KNF: f_store_ps:
; KNF: movl $1, %{{[a-z]*}}
; KNF: vkmov %{{[a-z]*}}, %k{{[0-9]*}}
; KNF: vpackstoreld   %{{v[0-9]+}}{sint16}, (%{{[a-z]*}}){nt}{%k{{[0-9]*}}}
entry:
; 5 = down conversion to sint, 1 = store one element, 1 = non-temporal
  call void @llvm.x86.mic.store.ps(i8 * %arg0, <16 x float> %arg1, i32 5, i32 1, i32 1)
  ret void 
}

define void @f_store_ps2(i8 * %arg0, <16 x float> %arg1) {
; KNF: f_store_ps2:
; KNF: vstored   %{{v[0-9]+}}{sint16}, (%{{[a-z]*}}){nt}
entry:
;  5 = down conversion to sint, 0 = store 16 element, 1 = non-temporal
  call void @llvm.x86.mic.store.ps(i8 * %arg0, <16 x float> %arg1, i32 5, i32 0, i32 1)
  ret void
}


