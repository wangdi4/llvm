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


define void @andnk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                   <8 x double> %c) nounwind ssp {
entry:
; KNF: andnk
; KNF: vkandn
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask1n = xor <8 x i1> %mask1, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %mask = and <8 x i1> %mask1n, %mask2
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

; This is a performance bug in PCG: constants are passed thru memory to GPRs instead of 'movl'
define void @andconst(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNF: andconst
; KNF: movl     $85, %e[[R1:[a-z]+]]
; KNF: vkmov    %e[[R1]], [[R2:%k[0-9]+]]
; KNF: vkand    [[R2]], {{%k[0-9]+}}
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = and <8 x i1> %mask1, <i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false>
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @andnrk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                    <8 x double> %c) nounwind ssp {
entry:
; KNF: andnrk
; KNF: vkandnr
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %mask2n = xor <8 x i1> %mask2, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %mask = and <8 x i1> %mask1, %mask2n
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void

define void @xnork(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                   <8 x double> %c) nounwind ssp {
entry:
; KNF: xnork
; KNF: vkxnor
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask2 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %b, <8 x double> %c )
  %xor = xor <8 x i1> %mask1, %mask2
  %mask = xor <8 x i1> %xor, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @notk(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNF: notk
; KNF: vknot
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = xor <8 x i1> %mask1, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define void @loadones(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                      <8 x double> %c) nounwind ssp {
entry:
; KNF: loadones
; KNF: vkxnor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>, <8 x double> %a)
  ret void
}

define <8 x i1> @cmpeq(<8 x i1> %m1, <8 x i1> %m2) nounwind ssp {
; KNF: cmpeq:
; KNF: vkxnor
  %mask = icmp eq <8 x i1> %m1, %m2
  ret <8 x i1> %mask
}

define void @and1(<8 x double>* %p, <8 x double> %a, <8 x double> %b,
                  <8 x double> %c) nounwind ssp {
entry:
; KNF: and1
; KNF: movl	$1
; KNF: vkand
  %mask1 = call <8 x i1> @llvm.x86.mic.cmpeq.pd( <8 x double> %a, <8 x double> %b )
  %mask = and <8 x i1> %mask1, <i1 -1, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0>
  %ptr = bitcast <8 x double>* %p to i8*
  call void @llvm.x86.mic.mask.store.pd(i8* %ptr, <8 x i1> %mask, <8 x double> %a)
  ret void
}

define <16 x i1> @concat2(<8 x i1> %a, <8 x i1> %b) nounwind ssp {
entry:
; KNF: concat2
; KNF: vkmovlhb
  %m = shufflevector <8 x i1> %a, <8 x i1> %b, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15,
                                                           i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <16 x i1> %m
}
