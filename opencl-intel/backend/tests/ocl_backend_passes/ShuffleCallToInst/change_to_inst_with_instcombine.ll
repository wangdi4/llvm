; RUN: opt -shuffle-call-to-inst -instcombine -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK: @sample_test
define void @sample_test(<4 x i32> %x, <4 x i32> addrspace(1)* %p1) nounwind {
entry:
  %call = call <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32> %x, <4 x i32> <i32 3, i32 2, i32 1, i32 0>) nounwind readnone
  store <4 x i32> %call, <4 x i32> addrspace(1)* %p1
  ret void
}

declare <4 x i32> @_Z7shuffleDv4_iDv4_j(<4 x i32>, <4 x i32>) nounwind readnone


; no calls should remain
; CHECK-NOT:    call