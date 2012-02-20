; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.pq(i8*, <8 x i1>, <8 x i64>)

define void @cmpeq(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpeq
; KNF: vcmppi	{eq},
; KNF: vcmppi	{eq},
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp eq <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

define void @cmpgt(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpgt
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nle},
; KNF: vkor
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp sgt <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

define void @cmpge(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpge
; KNF: vcmppi	{nle},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{nlt},
; KNF: vkor
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp sge <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

define void @cmplt(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmplt
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{lt},
; KNF: vkor
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp slt <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

define void @cmple(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmple
; KNF: vcmppi	{lt},
; KNF: vcmppi	{eq},
; KNF: vcmppu	{le},
; KNF: vkor
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp sle <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

define void @cmpne(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNF: @cmpne
; KNF: vcmppi	{neq},
; KNF: vcmppi	{neq},
; KNF: vkor
; KNF: vstoreq	%v0, (%rdi){%k1}
  %mask = icmp ne <8 x i64> %a, %b
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, <8 x i1> %mask, <8 x i64> %a)
  ret void
}

