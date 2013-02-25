; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;
;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.pi(i8*, i16, <16 x i32>, i32, i32)

define void @cmpugt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ugt <16 x i32> %a, %b
  %maski = bitcast <16 x i1> %mask to i16
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, i16 %maski, <16 x i32> %a, i32 0, i32 0)
  ret void
}

define void @cmpuge(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp uge <16 x i32> %a, %b
  %maski = bitcast <16 x i1> %mask to i16
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, i16 %maski, <16 x i32> %a, i32 0, i32 0)
  ret void
}

define void @cmpult(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ult <16 x i32> %a, %b
  %maski = bitcast <16 x i1> %mask to i16
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, i16 %maski, <16 x i32> %a, i32 0, i32 0)
  ret void
}

define void @cmpule(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ule <16 x i32> %a, %b
  %maski = bitcast <16 x i1> %mask to i16
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, i16 %maski, <16 x i32> %a, i32 0, i32 0)
  ret void
}


