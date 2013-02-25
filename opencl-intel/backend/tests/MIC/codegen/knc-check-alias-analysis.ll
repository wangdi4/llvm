; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

;

define void @checkScheduling(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %aout, i32* noalias nocapture %bout) {
; KNC: checkScheduling:
; KNC: vmovaps
; KNC: vmovaps
; KNC: vmovdqa32
; KNC: vmovdqa32
  %firstVec = bitcast i32* %a to <16 x i32>*
  %first = load <16 x i32>* %firstVec, align 64
  %firstStoreVec = bitcast i32* %aout to <16 x i32>*
  store <16 x i32> %first, <16 x i32>* %firstStoreVec, align 64

  %secondVec = bitcast i32* %b to <16 x i32>*
  %second = load <16 x i32>* %secondVec, align 64
  %secondStoreVec = bitcast i32* %bout to <16 x i32>*
  store <16 x i32> %second, <16 x i32>* %secondStoreVec, align 64

  ret void
}

define void @checkRedundantLoadNStoreRemoval(i32* noalias nocapture %a, i32* noalias nocapture %aout, i32* noalias nocapture %bout) {
; KNC: checkRedundantLoadNStoreRemoval:
; KNC: vmovaps
; KNC: vmovdqa32
; KNC: vmovdqa32
  %firstVec = bitcast i32* %a to <16 x i32>*
  %first = load <16 x i32>* %firstVec, align 64

  %firstStoreVec = bitcast i32* %aout to <16 x i32>*
  store <16 x i32> %first, <16 x i32>* %firstStoreVec, align 64

  %secondVec = bitcast i32* %a to <16 x i32>*
  %second = load <16 x i32>* %secondVec, align 64
  
  %firstStoreVec1 = bitcast i32* %aout to <16 x i32>*
  store <16 x i32> %second, <16 x i32>* %firstStoreVec1, align 64

  %secondStoreVec = bitcast i32* %bout to <16 x i32>*
  store <16 x i32> %second, <16 x i32>* %secondStoreVec, align 64

  ret void
}

define void @checkSchedulingAddressSpace(i32 addrspace(1)*  %a, i32* %b, i32 addrspace(1)* %aout, i32* %bout) {
; KNC: checkSchedulingAddressSpace:
; KNC: vmovaps
; KNC: vmovaps
; KNC: vmovdqa32
; KNC: vmovdqa32
  %firstVec = bitcast i32 addrspace(1)* %a to <16 x i32> addrspace(1)*
  %first = load <16 x i32> addrspace(1)* %firstVec, align 64
  %firstStoreVec = bitcast i32 addrspace(1)* %aout to <16 x i32> addrspace(1)*
  store <16 x i32> %first, <16 x i32> addrspace(1)* %firstStoreVec, align 64

  %secondVec = bitcast i32* %b to <16 x i32>*
  %second = load <16 x i32>* %secondVec, align 64
  %secondStoreVec = bitcast i32* %bout to <16 x i32>*
  store <16 x i32> %second, <16 x i32>* %secondStoreVec, align 64

  ret void
}

