; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare <8 x i1> @llvm.x86.mic.cmpeq.pd(<8 x double>, <8 x double>)
declare void @llvm.x86.mic.mask.store.pd(i8*, <8 x i1>, <8 x double>)

define void @andk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNF: andk
; KNF: vkand
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = and <8 x i1> %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @ork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                 <8 x double> %c) nounwind ssp {
entry:
; KNF: ork
; KNF: vkor
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = or <8 x i1> %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @xork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNF: xork
; KNF: vkxor
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = xor <8 x i1> %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @loadzero(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNF: loadzero
; KNF: vkxor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> <i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0>, <8 x double> %a)
  ret void
}

define void @sel(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNF: sel
; KNF: vkand{{n?}}
; KNF: vkand{{n?}}
; KNF: vkor
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %c )
  %mask3 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = select <8 x i1> %mask1, <8 x i1> %mask2, <8 x i1> %mask3
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)

  ret void
}

define <8 x i1> @subvec(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNF: subvec
; KNF: vkmov %k2, %k1
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i1> %m
}

define <8 x i1> @subvec2(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNF: subvec2
; KNF: vkswapb
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i1> %m
}

define <8 x i1> @subvec3(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNF: subvec3
; Just make sure this compiles, the code is going to be a bunch of movs and shifts
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8>
  ret <8 x i1> %m
}

define <16 x i1> @concat(<8 x i1> %a, <8 x i1> %b) nounwind ssp {
entry:
; KNF: concat
; KNF: vkmovlhb
  %m = shufflevector <8 x i1> %a, <8 x i1> %b, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
                                                           i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i1> %m
}

define <8 x i1> @cmpne(<8 x i1> %m1, <8 x i1> %m2) nounwind ssp {
; KNF: cmpne:
; KNF: vkxor
  %mask = icmp ne <8 x i1> %m1, %m2
  ret <8 x i1> %mask
}
