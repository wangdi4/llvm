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

declare <16 x i1> @llvm.x86.mic.cmpeq.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmplt.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmple.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmpunord.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmpneq.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmpnlt.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmpnle.ps(<16 x float>, <16 x float>)
declare <16 x i1> @llvm.x86.mic.cmpord.ps(<16 x float>, <16 x float>)
declare void @llvm.x86.mic.mask.store.ps(i8*, <16 x i1>, <16 x float>)

define void @cmpoeq(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{eq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpeqps	%zmm1, %zmm0, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp oeq <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpogt(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{lt}, %v0, %v1, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpltps	%zmm0, %zmm1, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp ogt <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpoge(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{le}, %v0, %v1, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpleps	%zmm0, %zmm1, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp oge <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpolt(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpltps	%zmm1, %zmm0, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp olt <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpole(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpleps	%zmm1, %zmm0, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp ole <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpone(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{ord}, %v1, %v0, %k
; KNF: vcmpps	{neq}, %v1, %v0, %k
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpordps	%zmm1, %zmm0, %k
; KNC: vcmpneqps	%zmm1, %zmm0, %k
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp one <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpord(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{ord}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpordps	%zmm1, %zmm0, %k
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = fcmp ord <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpueq(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{unord}, %v1, %v0, %k
; KNF: vcmpps	{eq}, %v1, %v0, %k
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpunordps	%zmm1, %zmm0, %k
; KNC: vcmpeqps		%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp ueq <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpugt(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnleps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp ugt <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpuge(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnltps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp uge <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpult(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nle}, %v0, %v1, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnleps	%zmm0, %zmm1, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp ult <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpule(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nlt}, %v0, %v1, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnltps	%zmm0, %zmm1, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp ule <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpune(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{neq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpneqps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp une <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @cmpunord(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{unord}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpunordps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = fcmp uno <16 x float> %a, %b
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpeq(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{eq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpeqps	%zmm1, %zmm0, %k1
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmplt(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{lt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpltps	%zmm1, %zmm0, %k
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmplt.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmple(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{le}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpleps	%zmm1, %zmm0, %k
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmple.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpunord(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{unord}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpunordps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpunord.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpneq(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{neq}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpneqps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpneq.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpnlt(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nlt}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnltps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnlt.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpnle(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{nle}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpnleps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpnle.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpord(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{ord}, %v1, %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpordps	%zmm1, %zmm0, %k
; KNC: vmovaps		%zmm0, (%rdi){%k1}
  %mask = call <16 x i1> @llvm.x86.mic.cmpord.ps( <16 x float> %a, <16 x float> %b )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @int_cmpmem(<16 x float>* %p, <16 x float> %a, <16 x float> %b) nounwind ssp {
entry:
; KNF: vcmpps	{eq}, (%rdi), %v0, %k1
; KNF: vstored	%v0, (%rdi){%k1}
;
; KNC: vcmpeqps	(%rdi), %zmm0, %k
; KNC: vmovaps	%zmm0, (%rdi){%k1}
  %load = load <16 x float>* %p
  %mask = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %load )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}
