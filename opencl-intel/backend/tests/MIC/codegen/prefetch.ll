; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.prefetch1(i8 *, i8)
declare void @llvm.x86.mic.prefetch2(i8 *, i8)

define void @prefetch1(<16 x float>* %p) nounwind readnone ssp {
; KNF: vprefetch1 $3, {{\(%[a-z]+\)}}
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch1(i8* %ptr, i8 3)
  ret void
}

define void @prefetch2(<16 x float>* %p) nounwind readnone ssp {
; KNF: vprefetch2 $3, {{\(%[a-z]+\)}}
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch2(i8* %ptr, i8 3)
  ret void
}

define void @prefetch3(<16 x float>* %p) nounwind readnone ssp {
; KNF: vprefetch2 $0, {{192\(%[a-z]+\)}}
  %ptr = getelementptr <16 x float>* %p, i32 3
  %ptr2 = bitcast <16 x float>* %ptr to i8*
  call void @llvm.x86.mic.prefetch2(i8* %ptr2, i8 0)
  ret void
}
