; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.cvt.pi2ps(<16 x i32>, i32)

define <16 x float> @f_cvt_pi2ps(<16 x i32> %arg0, i32 %arg1) {
; KNF: f_cvt_pi2ps:
; KNF: cvt
entry:
  %ret = call <16 x float> @llvm.x86.mic.cvt.pi2ps(<16 x i32> %arg0, i32 %arg1)

 ret <16 x float> %ret
}

