; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.mask.addsetc.pi(<16 x i32>, i16, i16, <16 x i32>, i8 *)

define <16 x i32> @f_mask_addsetc_pi(<16 x i32> %arg0, i16 %arg1, i16 %arg2, <16 x i32> %arg3, i8 * %arg4) {
; KNF: f_mask_addsetc_pi:
; KNF: vaddsetcpi %v{{[0-9]*}}, %k{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
entry:
  %ret = call <16 x i32> @llvm.x86.mic.mask.addsetc.pi(<16 x i32> %arg0, i16 %arg1, i16 %arg2, <16 x i32> %arg3, i8 * %arg4)

 ret <16 x i32> %ret
}

