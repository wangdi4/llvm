; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.scatter.pi(i8 *, i16, <16 x i32>, <16 x i32>, i32, i32, i32)

define void @f_mask_scatter_pi(i8 * %arg0, i16 %arg1, <16 x i32> %arg2, <16 x i32> %arg3) {
; KNF: f_mask_scatter_pi:
; KNF: vscatterd %v{{[0-9]*}}, (%{{[a-z]*}},%v{{[0-9]*}},4){%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  call void @llvm.x86.mic.mask.scatter.pi(i8 * %arg0, i16 %arg1, <16 x i32> %arg2, <16 x i32> %arg3, i32 0, i32 4, i32 0)

 ret void 
}

