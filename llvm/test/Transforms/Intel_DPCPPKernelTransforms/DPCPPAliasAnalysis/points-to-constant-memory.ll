; RUN: opt -passes=instcombine -aa-pipeline=dpcpp-kernel-aa -S %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-aa -dpcpp-kernel-aa-wrapper -instcombine -S %s | FileCheck %s

; This test checks that store to constant memory is removed by instcombine.

; CHECK-NOT: store

define void @test(i8 addrspace(2)* %c) {
  store i8 0, i8 addrspace(2)* %c
  ret void
}
