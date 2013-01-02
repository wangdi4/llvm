; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc | FileCheck %s -check-prefix=KNC

;
;

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.cmpeq.pd(<8 x double>, <8 x double>)
declare void @llvm.x86.mic.mask.store.pd(i8*, i8, <8 x double>, i32, i32)


define void @andnk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                   <8 x double> %c) nounwind ssp {
entry:
; KNC: andnk
; KNC: kandn
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask1n = xor i8 %mask1, -1
  %mask = and i8 %mask1n, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

; This is a performance bug in PCG: constants are passed thru memory to GPRs instead of 'movl'
define void @andconst(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNF: andconst
; KNF: movl     $85, %e[[R1:[a-z]+]]
; KNF: vkmov    %e[[R1]], [[R2:%k[0-9]+]]
; KNF: kand    [[R2]], {{%k[0-9]+}}
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = and i8 %mask1, -86 ; 10101010
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @andnrk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                    <8 x double> %c) nounwind ssp {
entry:
; KNF: andnrk
; KNF: vkandnr
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask2n = xor i8 %mask2, -1 ; 11111111
  %mask = and i8 %mask1, %mask2n
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}
define void @xnork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                   <8 x double> %c) nounwind ssp {
entry:
; KNC: xnork
; KNC: kxnor
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %xor = xor i8 %mask1, %mask2
  %mask = xor i8 %xor, -1 ; 11111111
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @notk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNC: notk
; KNC: knot
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = xor i8 %mask1, -1 ; 11111111
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define void @loadones(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNC: loadones
; KNC: kxnor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 -1, <8 x double> %a, i32 0, i32 0)
  ret void
}

define <8 x i1> @cmpeq(<8 x i1> %m1, <8 x i1> %m2) nounwind ssp {
; KNC: cmpeq:
; KNC: kxnor
  %mask = icmp eq <8 x i1> %m1, %m2
  ret <8 x i1> %mask
}

define void @and1(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNC: and1
; KNC: movl $1
; KNC: kand
  %mask1 = call i8 @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = and i8 %mask1, -128 ; 10000000
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, i8 %mask, <8 x double> %a, i32 0, i32 0)
  ret void
}

define <16 x i1> @concat2(<8 x i1> %a, <8 x i1> %b) nounwind ssp {
entry:
; KNC: concat2
; KNC: kmovlhb
  %m = shufflevector <8 x i1> %a, <8 x i1> %b, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
                                                           i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <16 x i1> %m
}

