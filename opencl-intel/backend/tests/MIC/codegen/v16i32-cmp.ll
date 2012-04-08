; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare <16 x i1> @llvm.x86.mic.cmpeq.pi(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmplt.pi(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmple.pi(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpneq.pi(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpnlt.pi(<16 x i32>, <16 x i32>)
declare <16 x i1> @llvm.x86.mic.cmpnle.pi(<16 x i32>, <16 x i32>)
declare void @llvm.x86.mic.mask.store.pi(i8*, <16 x i1>, <16 x i32>)


define void @cmpeq(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{eq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp eq <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpgt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp sgt <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpge(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp sge <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmplt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp slt <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmple(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp sle <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @cmpne(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{neq}, %v1, %v0, %k
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = icmp ne <16 x i32> %a, %b
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpeq(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{eq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmplt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmplt.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmple(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmple.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpneq(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{neq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpneq.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpnlt(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnlt.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpnle(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnle.pi( <16 x i32> %a, <16 x i32> %b )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

define void @int_cmpmem(<16 x i32>* %p, <16 x i32> %a, <16 x i32> %b) nounwind ssp {
entry:
; KNF: vcmppi	{eq}, (%rdi), %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
  %load = load <16 x i32>* %p 
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.pi( <16 x i32> %a, <16 x i32> %load )
  %ptr = bitcast <16 x i32>* %p to i8*
  call void @llvm.x86.mic.mask.store.pi(i8* %ptr, <16 x i1> %mask, <16 x i32> %a)
  ret void
}

