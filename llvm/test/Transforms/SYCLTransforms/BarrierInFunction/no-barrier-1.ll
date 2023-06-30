; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass
;; The case: kernel "main" with no barrier instruction
;; The expected result:
;;      1. No call to @dummy_barrier.() at the begining of the kernel "main"
;;      2. No call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the kernel "main"
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind !no_barrier_path !1 {
  %y = xor i32 %x, %x
  ret void
; CHECK-NOT: @dummy_barrier.()
; CHECK: %y = xor i32 %x, %x
; CHECK-NOT: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}

!sycl.kernels = !{!0}
!opencl.disabled.FP_CONTRACT = !{}

!0 = !{ptr @main}
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
