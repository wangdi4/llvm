; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK:  @copy
; CHECK:  %0 = load i32, i32 addrspace(1)* %arrayidx
; CHECK-NEXT:  call void @_Z12atomic_storePU3AS3Vii(i32 addrspace(3)* %add.ptr, i32 %0)

; This test checks that genericAddressSpaceDynamicResolution pass does not consider function pointer address space when resolving the common address space of a built-in.

; ModuleID = 'BugCSSD100017424'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @copy(i32 addrspace(1)* nocapture %oldValues, i32 addrspace(3)* %destMemory) nounwind {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0) nounwind readnone
  %add.ptr = getelementptr inbounds i32, i32 addrspace(3)* %destMemory, i64 %call
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %oldValues, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !1
  tail call void @_Z12atomic_storePU3AS3Vii(i32 addrspace(3)* %add.ptr, i32 %0) nounwind
  ret void
}

declare i64 @_Z12get_local_idj(i32) nounwind readnone

declare void @_Z12atomic_storePU3AS3Vii(i32 addrspace(3)*, i32)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*)* @copy}
!1 = !{!"int", !2}
!2 = !{!"omnipotent char", !3}
!3 = !{!"Simple C/C++ TBAA"}
