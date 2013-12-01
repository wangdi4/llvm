; RUN: opt -shuffle-call-to-inst -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, <8 x i32> %y, <2 x i8> %c, <2 x double> %d, <4 x i32> addrspace(1)* %p1, <8 x i32> addrspace(1)* %p2, <16 x i32> addrspace(1)* %p3, <8 x i32> addrspace(1)* %p4, <2 x i8> addrspace(1)* %p5, <2 x double> addrspace(1)* %p6) nounwind {
entry:
  %call1 = call <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <4 x i32> %call1, <4 x i32> addrspace(1)* %p1

  %call2 = call <8 x i32> @_Z7shuffleDv8_iDv8_j(<8 x i32> %y, <8 x i32> <i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <8 x i32> %call2, <8 x i32> addrspace(1)* %p2

  %call3 = call <16 x i32> @_Z7shuffleDv4_iDv16_j(<4 x i32> %x, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <16 x i32> %call3, <16 x i32> addrspace(1)* %p3

  %call4 = call <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32> %x, <8 x i32> %y) nounwind readnone
  store <8 x i32> %call4, <8 x i32> addrspace(1)* %p4

  %call5 = call <2 x i8> @_Z7shuffleDv2_cDv2_h(<2 x i8> %c, <2 x i8> <i8 1, i8 0>) nounwind readnone
  store <2 x i8> %call5, <2 x i8> addrspace(1)* %p5

  %call6 = call <2 x double> @_Z7shuffleDv2_dDv2_m(<2 x double> %d, <2 x i64> <i64 1, i64 0>) nounwind readnone
  store <2 x double> %call6, <2 x double> addrspace(1)* %p6
  ret void
}

declare <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32>, <4 x i32>) nounwind readnone
declare <8 x i32> @_Z7shuffleDv8_iDv8_j(<8 x i32>, <8 x i32>) nounwind readnone
declare <16 x i32> @_Z7shuffleDv4_iDv16_j(<4 x i32>, <16 x i32>) nounwind readnone
declare <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32>, <8 x i32>) nounwind readnone
declare <2 x i8> @_Z7shuffleDv2_cDv2_h(<2 x i8>, <2 x i8>) nounwind readnone
declare <2 x double> @_Z7shuffleDv2_dDv2_m(<2 x double>, <2 x i64>) nounwind readnone

; change the first 3 shuffle calls
; CHECK:        [[NEW_SHUFFLE:%[a-zA-Z0-9]+]] = shufflevector <4 x i32> %x, <4 x i32> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <4 x i32> [[NEW_SHUFFLE]], <4 x i32> addrspace(1)* %p1
; no calls should remain
; CHECK-NOT:    call <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>)

; CHECK:        [[NEW_SHUFFLE1:%[a-zA-Z0-9]+]] = shufflevector <8 x i32> %y, <8 x i32> undef, <8 x i32> <i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <8 x i32> [[NEW_SHUFFLE1]], <8 x i32> addrspace(1)* %p2
; no calls should remain
; CHECK-NOT:    call <8 x i32> @_Z7shuffleDv8_iDv8_j(<8 x i32> %y, <8 x i32> <i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>)

; CHECK:        [[NEW_SHUFFLE2:%[a-zA-Z0-9]+]] = shufflevector <4 x i32> %x, <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0>
; CHECK:        store <16 x i32> [[NEW_SHUFFLE2]], <16 x i32> addrspace(1)* %p3
; no calls should remain
; CHECK-NOT:    call <16 x i32> @_Z7shuffleDv4_iDv16_j(<4 x i32> %x, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0, i32 0, i32 1, i32 2, i32 3, i32 3, i32 2, i32 1, i32 0>)

; this should not change
; CHECK:        %call4 = call <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32> %x, <8 x i32> %y)
; CHECK:        store <8 x i32> %call4, <8 x i32> addrspace(1)* %p4

; checking different mask element sizes -- should remain i32
; CHECK:        [[NEW_SHUFFLE3:%[a-zA-Z0-9]+]] = shufflevector <2 x i8> %c, <2 x i8> undef, <2 x i32> <i32 1, i32 0>
; CHECK:        store <2 x i8> [[NEW_SHUFFLE3]], <2 x i8> addrspace(1)* %p5
; no calls should remain
; CHECK-NOT:    call <8 x i32> @_Z7shuffleDv4_iDv8_j(<4 x i32> %x, <8 x i32> %y)

; CHECK:        [[NEW_SHUFFLE4:%[a-zA-Z0-9]+]] = shufflevector <2 x double> %d, <2 x double> undef, <2 x i32> <i32 1, i32 0>
; CHECK:        store <2 x double> [[NEW_SHUFFLE4]], <2 x double> addrspace(1)* %p6
; no calls should remain
; CHECK-NOT:    call <2 x double> @_Z7shuffleDv2_dDv2_m(<2 x double> %d, <2 x i64> <i64 1, i64 0>)