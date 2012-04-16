; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.mask.gather.pq(<8 x i64>, i8, <16 x i32>, i8 *, i32, i32, i32)

define <8 x i64> @f_mask_gather_pq(<8 x i64> %arg0, i8 %arg1, <16 x i32> %arg2, i8 * %arg3) {
; KNF: f_mask_gather_pq:
; KNF: vshuf128x32 $80, $80, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vshuf128x32 $250, $80, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vgatherd (%{{[a-z]*}},%v{{[0-9]*}},4), %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
entry:
  %ret = call <8 x i64> @llvm.x86.mic.mask.gather.pq(<8 x i64> %arg0, i8 %arg1, <16 x i32> %arg2, i8 * %arg3, i32 0, i32 8, i32 0)

 ret <8 x i64> %ret
}

