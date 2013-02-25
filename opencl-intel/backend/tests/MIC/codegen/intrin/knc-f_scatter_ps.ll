; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatter.ps(i8 *, <16 x i32>, <16 x float>, i32, i32, i32)

define void @f_scatter_ps(i8 * %arg0, <16 x i32> %arg1, <16 x float> %arg2) {
; KNF: f_scatter_ps:
; KNF: vkxnor %k{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vscatterd %v{{[0-9]*}}, (%{{[a-z]*}},%v{{[0-9]*}},4){%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  call void @llvm.x86.mic.scatter.ps(i8 * %arg0, <16 x i32> %arg1, <16 x float> %arg2, i32 0, i32 4, i32 0)

 ret void 
}

