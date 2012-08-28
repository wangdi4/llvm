; RUN: opt -shuffle-call-to-inst -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, <8 x i32> %y, <4 x i32> addrspace(1)* %p1, <8 x i32> addrspace(1)* %p2, <16 x i32> addrspace(1)* %p3, <8 x i32> addrspace(1)* %p4) nounwind {
entry:
  %call1 = call <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <4 x i32> %call1, <4 x i32> addrspace(1)* %p1

  %call2 = call <8 x i32> @_Z7shuffleDv8_iDv8_j(<8 x i32> %y, <8 x i32> <i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <8 x i32> %call2, <8 x i32> addrspace(1)* %p2

  %call3 = call <16 x i32> @_Z7shuffleDv4_iDv16_j(<4 x i32> %x, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <16 x i32> %call3, <16 x i32> addrspace(1)* %p3

  %call4 = call <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32> %x, <8 x i32> %y) nounwind readnone
  store <8 x i32> %call4, <8 x i32> addrspace(1)* %p4
  ret void
}

declare <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32>, <4 x i32>) nounwind readnone
declare <8 x i32> @_Z7shuffleDv8_iDv8_j(<8 x i32>, <8 x i32>) nounwind readnone
declare <16 x i32> @_Z7shuffleDv4_iDv16_j(<4 x i32>, <16 x i32>) nounwind readnone
declare <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32>, <8 x i32>) nounwind readnone


; change the first 3 shuffle calls
; CHECK:        [[NEW_SHUFFLE:%[a-zA-Z0-9]+]] = shufflevector <4 x i32> %x, <4 x i32> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <4 x i32> [[NEW_SHUFFLE]], <4 x i32> addrspace(1)* %p1

; CHECK:        [[NEW_SHUFFLE1:%[a-zA-Z0-9]+]] = shufflevector <8 x i32> %y, <8 x i32> undef, <8 x i32> <i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <8 x i32> [[NEW_SHUFFLE1]], <8 x i32> addrspace(1)* %p2

; CHECK:        [[NEW_SHUFFLE2:%[a-zA-Z0-9]+]] = shufflevector <4 x i32> %x, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <16 x i32> [[NEW_SHUFFLE2]], <16 x i32> addrspace(1)* %p3

; this should not change
; CHECK:        %call4 = call <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32> %x, <8 x i32> %y) nounwind readnone
; CHECK:        store <8 x i32> %call4, <8 x i32> addrspace(1)* %p4
