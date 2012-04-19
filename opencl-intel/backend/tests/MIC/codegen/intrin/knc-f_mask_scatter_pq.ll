; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.scatter.pq(i8 *, i8, <16 x i32>, <8 x i64>, i32, i32, i32)

define void @f_mask_scatter_pq(i8 * %arg0, i8 %arg1, <16 x i32> %arg2, <8 x i64> %arg3 ) {
; KNF: f_mask_scatter_pq:
; KNF: vshuf128x32 $80, $80, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vshuf128x32 $250, $80, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vscatterd %v{{[0-9]*}}, (%{{[a-z]*}},%v{{[0-9]*}},4){%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  call void @llvm.x86.mic.mask.scatter.pq(i8 * %arg0, i8 %arg1, <16 x i32> %arg2, <8 x i64> %arg3, i32 0, i32 8, i32 0)

 ret void 
}

