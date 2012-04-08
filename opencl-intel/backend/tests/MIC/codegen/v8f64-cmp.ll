; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

declare <8 x i1> @llvm.x86.mic.cmpeq.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmplt.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmple.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmpunord.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmpneq.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmpnlt.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmpnle.pd(<8 x double>, <8 x double>)
declare <8 x i1> @llvm.x86.mic.cmpord.pd(<8 x double>, <8 x double>)
declare void @llvm.x86.mic.mask.store.pd(i8*, <8 x i1>, <8 x double>)

define void @cmpoeq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{eq}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpeqpd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp oeq <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpogt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{lt}, %v0, %v1, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpltpd	%zmm0, %zmm1, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp ogt <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpoge(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{le}, %v0, %v1, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmplepd	%zmm0, %zmm1, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp oge <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpolt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{lt}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpltpd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp olt <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpole(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{le}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmplepd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp ole <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpone(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{ord}, %v1, %v0, %k
; KNF: vcmppd	{neq}, %v1, %v0, %k
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpordpd	%zmm1, %zmm0, %k
; KNC: vcmpneqpd	%zmm1, %zmm0, %k
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp one <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{ord}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpordpd	%zmm1, %zmm0, %k
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = fcmp ord <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpueq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{unord}, %v1, %v0, %k
; KNF: vcmppd	{eq}, %v1, %v0, %k
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpunordpd	%zmm1, %zmm0, %k
; KNC: vcmpeqpd		%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp ueq <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpugt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nle}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnlepd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp ugt <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpuge(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nlt}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnltpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp uge <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpult(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nle}, %v0, %v1, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnlepd	%zmm0, %zmm1, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp ult <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpule(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nlt}, %v0, %v1, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnltpd	%zmm0, %zmm1, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp ule <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpune(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{neq}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpneqpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp une <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @cmpunord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{unord}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpunordpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = fcmp uno <8 x double> %a, %b
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpeq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{eq}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpeqpd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmplt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{lt}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpltpd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmplt.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmple(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{le}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmplepd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmple.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpunord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{unord}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpunordpd	%zmm1, %zmm0, %k1
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpunord.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpneq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{neq}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpneqpd	%zmm1, %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpneq.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpnlt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nlt}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnltpd	%zmm1, %zmm0, %k1
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpnlt.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpnle(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{nle}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpnlepd	%zmm1, %zmm0, %k1
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpnle.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{ord}, %v1, %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpordpd	%zmm1, %zmm0, %k1
; KNC: vmovapd		%zmm0, (%rdi){%k1}
  %mask = call <8 x i1> @llvm.x86.mic.cmpord.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @int_cmpmem(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
; KNF: vcmppd	{eq}, (%rdi), %v0, %k1
; KNF: vstoreq	%v0, (%rdi){%k1}
;
; KNC: vcmpeqpd	(%rdi), %zmm0, %k1
; KNC: vmovapd	%zmm0, (%rdi){%k1}
  %load = load <8 x double>* %p
  %mask = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %load )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}
