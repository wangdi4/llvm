; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.cmpeq.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmplt.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmple.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmpunord.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmpneq.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmpnlt.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmpnle.pd(<8 x double>, <8 x double>)
declare i8 @llvm.x86.mic.cmpord.pd(<8 x double>, <8 x double>)
declare void @llvm.x86.mic.mask.store.pd(i8*, i8, <8 x double>, i32, i32)

define void @cmpoeq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpeqpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp oeq <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpogt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpltpd	%zmm0, %zmm1, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp ogt <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpoge(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmplepd	%zmm0, %zmm1, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp oge <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpolt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpltpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp olt <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpole(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmplepd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp ole <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpone(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpordpd	%zmm1, %zmm0, %k
; KNC: vcmpneqpd	%zmm1, %zmm0, %k
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp one <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpordpd	%zmm1, %zmm0, %k
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %bmask = fcmp ord <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpueq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpunordpd	%zmm1, %zmm0, %k
; KNC: vcmpeqpd		%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp ueq <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpugt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnlepd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp ugt <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpuge(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnltpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp uge <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpult(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnlepd	%zmm0, %zmm1, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp ult <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpule(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnltpd	%zmm0, %zmm1, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp ule <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpune(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpneqpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp une <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @cmpunord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpunordpd	%zmm1, %zmm0, %k
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %bmask = fcmp uno <8 x double> %a, %b
  %mask = bitcast <8 x i1> %bmask to i8
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpeq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpeqpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmplt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpltpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmplt.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmple(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmplepd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmple.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpunord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpunordpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpunord.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpneq(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpneqpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpneq.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpnlt(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnltpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpnlt.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpnle(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpnlepd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpnle.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpord(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpordpd	%zmm1, %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd		%zmm0, (%rdi)[[K]]
  %mask = call i8 @llvm.x86.mic.cmpord.pd( <8 x double> %a, <8 x double> %b )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @int_cmpmem(<8 x double>* %p, <8 x double> %a, <8 x double> %b) nounwind ssp {
entry:
;
; KNC: vcmpeqpd	(%rdi), %zmm0, [[K:%k[0-9]+]]
; KNC: vmovapd	%zmm0, (%rdi)[[K]]
  %load = load <8 x double>* %p
  %mask = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %load )
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}
