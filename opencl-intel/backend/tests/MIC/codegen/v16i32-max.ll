; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8
declare <16 x i32> @llvm.x86.mic.max.pi(<16 x i32>, <16 x i32>)

define <16 x i32> @max1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; KNF: vmaxpi {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %max = call <16 x i32> @llvm.x86.mic.max.pi(<16 x i32> %a, <16 x i32> %b)
  ret <16 x i32> %max
}

define <16 x i32> @max2(<16 x i32> %a, <16 x i32>* %p) nounwind readnone ssp {
entry:
; KNF: vmaxpi ({{%r[a-z0-9]+}}), {{%v[0-9]+}}, {{%v[0-9]+}}
  %b = load <16 x i32>* %p
  %max = call <16 x i32> @llvm.x86.mic.max.pi(<16 x i32> %a, <16 x i32> %b)
  ret <16 x i32> %max
}

