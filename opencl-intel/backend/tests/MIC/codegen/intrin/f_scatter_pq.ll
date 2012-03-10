; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatter.pq(i8 *, <8 x i64>, <8 x i64>, i32, i32, i32)

define void @f_scatter_pq(i8 * %arg0, <8 x i64> %arg1, <8 x i64> %arg2) {
; KNF: f_scatter_pq:
; KNF: vshuf128x32 $80, $80, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vshuf128x32 $250, $80, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vkxnor %k{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vscatterd %v{{[0-9]*}}, (%{{[a-z]*}},%v{{[0-9]*}},4){%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  call void @llvm.x86.mic.scatter.pq(i8 * %arg0, <8 x i64> %arg1, <8 x i64> %arg2, i32 0, i32 8, i32 0)

 ret void 
}

