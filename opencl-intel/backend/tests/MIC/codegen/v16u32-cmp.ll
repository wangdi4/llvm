; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare <16 x i1> @llvm.x86.mic.cmpeq.pu(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmplt.pu(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmple.pu(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpneq.pu(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpnlt.pu(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpnle.pu(<16 x i32>, <16 x i32>)
declare void @llvm.x86.mic.mask.store.pi(i8*, <16 x i1>, <16 x i32>)

define void @cmpugt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ugt <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpuge(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp uge <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpult(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ult <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpule(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ule <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpeq(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{eq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmplt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmplt.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmple(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmple.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpneq(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{neq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpneq.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpnlt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnlt.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpnle(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnle.pu( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpmem(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppu	{eq}, (%rdi), %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %load = load <16 x i32>* %p 
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.pu( <16 x i32> %a, <16 x i32> %load )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

