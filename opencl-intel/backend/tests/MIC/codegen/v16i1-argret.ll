; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare <16 x i1> @llvm.x86.mic.cmpeq.ps(<16 x float>, <16 x float>)
declare void @llvm.x86.mic.mask.store.ps(i8*, <16 x i1>, <16 x float>)

define void @andkarg(<16 x float>* %p, <16 x i1> %mask1, <16 x i1> %mask2,
                     <16 x float> %a) nounwind ssp {
entry:
; KNF: vkand [[R1:%k[12]+]], [[R2:%k[12]+]]
; KNF: vstored %v0, (%rdi){[[R2]]}
  %mask = and <16 x i1> %mask1, %mask2
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define <16 x i1> @andkret(<16 x i1> %mask1, <16 x i1> %mask2) nounwind ssp {
entry:
; KNF: vkand %k2, %k1
  %mask = and <16 x i1> %mask1, %mask2
  ret <16 x i1> %mask
}

define void @maskcall(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                      <16 x float> %c) nounwind ssp {
entry:
; KNF: vcmpps	{eq}, {{%v[012]}}, {{%v[012]}}, {{%k[12]}}
; KNF: vcmpps	{eq}, {{%v[012]}}, {{%v[012]}}, {{%k[12]}}
; KNF: callq	andkret
; KNF: vstored	%v0, (%rbx){%k1}
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask = call <16 x i1> @andkret( <16 x i1> %mask1, <16 x i1> %mask2 )
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}
