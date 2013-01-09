; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.cmpeq.pd(<8 x double>, <8 x double>)
declare void @llvm.x86.mic.mask.store.pd(i8*, i8, <8 x double>, i32, i32)

define void @andk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNC: andk
; KNC: vkand
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = and i8 %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @ork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                 <8 x double> %c) nounwind ssp {
entry:
; KNC: ork
; KNC: vkor
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = or i8 %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @xork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNC: xork
; KNC: vkxor
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask = xor i8 %mask1, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @loadzero(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNC: loadzero
; KNC: vkxor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 0, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @sel(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNC: sel
; KNC: vkand{{n?}}
; KNC: vkand{{n?}}
; KNC: vkor
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %c )
  %mask3 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  ;%bmask1 = bitcast i8 %mask1 to <8 x i1>
  ;%bmask2 = bitcast i8 %mask2 to <8 x i1>
  ;%bmask3 = bitcast i8 %mask3 to <8 x i1>
  ;%bmask = select <8 x i1> %bmask1, <8 x i1> %bmask2, <8 x i1> %bmask3
  ;%mask = bitcast <8 x i1> %bmask to i8
  %and12 = and i8 %mask1, %mask2
  %and13 = and i8 %mask1, %mask3
  %not_and13 = xor i8 255, %and13
  %mask = and i8 %and12, %not_and13
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)

  ret void
}

define <8 x i1> @subvec(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNC: subvec
; KNC: vkmov %k2, %k1
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i1> %m
}

define <8 x i1> @subvec2(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNC: subvec2
; KNC: vkswapb
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <8 x i1> %m
}

define <8 x i1> @subvec3(<16 x i1> %a, <16 x i1> %b) nounwind ssp {
entry:
; KNC: subvec3
; Just make sure this compiles, the code is going to be a bunch of movs and shifts
  %m = shufflevector <16 x i1> %b, <16 x i1> %b, <8 x i32> <i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8>
  ret <8 x i1> %m
}

define <16 x i1> @concat(<8 x i1> %a, <8 x i1> %b) nounwind ssp {
entry:
; KNC: concat
; KNC: vkmovlhb
  %m = shufflevector <8 x i1> %a, <8 x i1> %b, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7,
                                                           i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  ret <16 x i1> %m
}

define <8 x i1> @cmpne(<8 x i1> %m1, <8 x i1> %m2) nounwind ssp {
; KNC: cmpne:
; KNC: vkxor
  %mask = icmp ne <8 x i1> %m1, %m2
  ret <8 x i1> %mask
}

