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

define void @andk(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                  <16 x float> %c) nounwind ssp {
entry:
; KNF: vkand
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask = and <16 x i1> %mask1, %mask2
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @andconst(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                      <16 x float> %c) nounwind ssp {
entry:
; KNF: movl     $21845, %e[[R1:[a-z]+]]
; KNF: vkmov    %e[[R1]], [[R2:%k[0-9]+]]
; KNF: vkand    [[R2]], {{%k[0-9]+}}
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask = and <16 x i1> %mask1, <i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false, i1 true, i1 false>
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @ork(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                 <16 x float> %c) nounwind ssp {
entry:
; KNF: vkor
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask = or <16 x i1> %mask1, %mask2
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @xork(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                  <16 x float> %c) nounwind ssp {
entry:
; KNF: vkxor
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask = xor <16 x i1> %mask1, %mask2
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @andnk(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                   <16 x float> %c) nounwind ssp {
entry:
; KNF: vkandn
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask1n = xor <16 x i1> %mask1, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %mask = and <16 x i1> %mask1n, %mask2
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @andnrk(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                    <16 x float> %c) nounwind ssp {
entry:
; KNF: vkandnr
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask2n = xor <16 x i1> %mask2, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %mask = and <16 x i1> %mask1, %mask2n
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @xnork(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                   <16 x float> %c) nounwind ssp {
entry:
; KNF: vkxnor
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %xor = xor <16 x i1> %mask1, %mask2
  %mask = xor <16 x i1> %xor, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @notk(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                  <16 x float> %c) nounwind ssp {
entry:
; KNF: vknot
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask = xor <16 x i1> %mask1, <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @and1(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                  <16 x float> %c) nounwind ssp {
entry:
; KNF: movl	$1
; KNF: vkand
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask = and <16 x i1> %mask1, <i1 -1, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0>
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)
  ret void
}

define void @loadzero(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                      <16 x float> %c) nounwind ssp {
entry:
; KNF: vkxor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> <i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0, i1 0>, <16 x float> %a)
  ret void
}

define void @loadones(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                      <16 x float> %c) nounwind ssp {
entry:
; KNF: vkxnor {{%k[0-9]+}}, {{%k[0-9]+}}
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> <i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1, i1 -1>, <16 x float> %a)
  ret void
}

define void @sel(<16 x float>* %p, <16 x float> %a, <16 x float> %b,
                      <16 x float> %c) nounwind ssp {
entry:
; KNF: vkand{{n?}}
; KNF: vkand{{n?}}
; KNF: vkor
  %mask1 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %b )
  %mask2 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %a, <16 x float> %c )
  %mask3 = call <16 x i1> @llvm.x86.mic.cmpeq.ps( <16 x float> %b, <16 x float> %c )
  %mask = select <16 x i1> %mask1, <16 x i1> %mask2, <16 x i1> %mask3
  %ptr = bitcast <16 x float>* %p to i8*
  call void @llvm.x86.mic.mask.store.ps(i8* %ptr, <16 x i1> %mask, <16 x float> %a)

  ret void
}

define <16 x i1> @cmpeq(<16 x i1> %m1, <16 x i1> %m2) nounwind ssp {
; KNF: cmpeq:
; KNF: vkxnor
  %mask = icmp eq <16 x i1> %m1, %m2
  ret <16 x i1> %mask
}

define <16 x i1> @cmpne(<16 x i1> %m1, <16 x i1> %m2) nounwind ssp {
; KNF: cmpne:
; KNF: vkxor
  %mask = icmp ne <16 x i1> %m1, %m2
  ret <16 x i1> %mask
}
