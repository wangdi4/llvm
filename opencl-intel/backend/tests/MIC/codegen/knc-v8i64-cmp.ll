; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s -check-prefix=KNC 
;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.store.pq(i8*, i8, <8 x i64>, i32, i32)

define void @cmpeq(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmpeq:
; KNC: vpcmpeqd
; KNC: vcmpneqpd
; KNC: vmovdqa64
  %mask = icmp eq <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

define void @cmpgt(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmpgt:
; KNC: vpcmpgtd
; KNC: vcmpneqpd
; KNC: vmovdqa64
  %mask = icmp sgt <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

define void @cmpge(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmpge:
; KNC: vpcmpgtd
; KNC: vcmpneqpd
; KNC: vmovdqa64
  %mask = icmp sge <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

define void @cmplt(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmplt:
; KNC: vpcmpltd
; KNC: vcmpneqpd
; KNC: vmovdqa64
  %mask = icmp slt <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

define void @cmple(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmple:
; KNC: vpcmpltd
; KNC: vcmpneqpd
; KNC: vmovdqa64
  %mask = icmp sle <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

define void @cmpne(<8 x i64>* %p, <8 x i64> %a, <8 x i64> %b) nounwind ssp {
entry:
; KNC: cmpne:
; KNC: vpcmpd
; KNC: vcmpneqpd
; KNC: vmovdqa64 
  %mask = icmp ne <8 x i64> %a, %b
  %imask = bitcast <8 x i1> %mask to i8
  %ptr = bitcast <8 x i64>* %p to i8*
  call void @llvm.x86.mic.mask.store.pq(i8* %ptr, i8 %imask, <8 x i64> %a, i32 0, i32 0)
  ret void
}

