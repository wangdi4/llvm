; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.load.pi(i8 *, i32, i32, i32)

define <16 x i32> @f_load_pi(i8 * %arg0) {
; KNF: f_load_pi:
; KNF: vloadd (%{{[a-z]+}}){sint16i}{4to16}{nt}
entry:
  %ret = call <16 x i32> @llvm.x86.mic.load.pi(i8 * %arg0, i32 4, i32 2, i32 1)

 ret <16 x i32> %ret
}

