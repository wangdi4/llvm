; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x i64> @llvm.x86.mic.min.pq(<8 x i64>, <8 x i64>)

define <8 x i64> @f_min_pq(<8 x i64> %arg0, <8 x i64> %arg1) {
; KNF: f_min_pq:
; KNF: vcmppu {eq}, %v{{[0-9]*}}, %v{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vcmppi {nle}, %v{{[0-9]*}}, %v{{[0-9]*}}, %k{{[0-9]*}}
; KNF: bitinterleave11 %{{[a-z0-9]*}}, %{{[a-z0-9]*}}
; KNF: quadmask16 %{{[a-z0-9]*}}, %{{[a-z0-9]*}}
entry:
  %ret = call <8 x i64> @llvm.x86.mic.min.pq(<8 x i64> %arg0, <8 x i64> %arg1)

 ret <8 x i64> %ret
}

