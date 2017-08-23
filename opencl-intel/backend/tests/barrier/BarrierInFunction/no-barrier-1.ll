; RUN: opt -B-BarrierInFunction -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass
;; The case: kernel "main" with no barrier instruction
;; The expected result:
;;      1. A call to @dummybarrier.() at the begining of the kernel "main"
;;      2. A call to @_Z7barrierj(LOCAL_MEM_FENCE) at the end of the kernel "main"
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
  %y = xor i32 %x, %x
  ret void
; CHECK: @dummybarrier.()
; CHECK: %y = xor i32 %x, %x
; CHECK: @_Z7barrierj(i32 1)
; CHECK: ret
}

!opencl.kernels = !{!0}
!opencl.disabled.FP_CONTRACT = !{}

!0 = !{void (i32)* @main}
