; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc | FileCheck %s


target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.prefetch(i8 *, i32)

define void @prefetch_l1_temporal_non-exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetch0 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 1)
  ret void
}

define void @prefetch_l1_temporal_exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetche0 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 5)
  ret void
}

define void @prefetch_l1_non-temporal_non-exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetchnta (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 0)
  ret void
}

define void @prefetch_l1_non-temporal_exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetchenta (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 4)
  ret void
}

define void @prefetch_l2_temporal_non-exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetch1 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 2)
  ret void
}

define void @prefetch_l2_temporal_exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetche1 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 6)
  ret void
}

define void @prefetch_l2_non-temporal_non-exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetch2 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 3)
  ret void
}

define void @prefetch_l2_non-temporal_exclusive(<16 x float>* %p) nounwind readnone ssp {
; CHECK: prefetche2 (%rdi)
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.prefetch(i8* %ptr, i32 7)
  ret void
}
