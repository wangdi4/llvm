; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc | FileCheck %s

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.prefetch(i8 *, i32)

define void @prefetch_mid_vector(<16 x float>* %p) nounwind readnone ssp {
; CHECK: vprefetch1 192(%rdi)
  %ptr = getelementptr <16 x float>* %p, i32 3
  %ptr2 = bitcast <16 x float>* %ptr to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr2, i32 2)
  ret void
}
