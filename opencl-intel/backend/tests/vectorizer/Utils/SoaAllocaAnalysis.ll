; RUN: opt -analyze -SoaAllocaAnalysis -verify %s -S -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check SoaAllocaAnalysis decides not to optimize alloca
;; that its address is stored in some memory.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; ModuleID = 'Program'

declare i32 @_Z13get_global_idj(i32) nounwind readnone

@testKernel.structWithPointers.2 = internal addrspace(3) global i32* null, align 4

define void @__Vectorized_.testKernel(i32 addrspace(1)* nocapture %results) nounwind {
entry:
  %PackedAlloca = alloca <4 x i32>, align 16
  %pint = alloca i32, align 4
  %call = call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %temp = insertelement <4 x i32> undef, i32 %call, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = add <4 x i32> %vector, <i32 0, i32 1, i32 2, i32 3>
  store i32* %pint, i32* addrspace(3)* @testKernel.structWithPointers.2, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %results, i32 %call
  store i32 1, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; CHECK: @__Vectorized_.testKernel
; CHECK: SAA-Start
; CHECK-NOT: %pint
; CHECK: __Vectorized_.testKernel
; CHECK: %PackedAlloca = alloca <4 x i32>, align 16 SR:[0] VR:[1] PR:[1]
; CHECK-NOT: %pint
; CHECK: SAA-End