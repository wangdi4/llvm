; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass
;; The case: kernel "main" and function "foo" with no barrier instruction,
;;    which is calling function "foo1" that contains barrier instruction
;; The expected result:
;;      1. A call to @dummy_barrier.() at the begining of the kernel "main"
;;      2. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) just before calling the function "foo"
;;      3. A call to @dummy_barrier.() just after calling the function "foo"
;;      4. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the kernel "main"
;;      1. A call to @dummy_barrier.() at the begining of the kernel "foo"
;;      2. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) just before calling the function "foo1"
;;      3. A call to @dummy_barrier.() just after calling the function "foo1"
;;      4. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the kernel "foo"
;;      5. A call to @dummy_barrier.() at the begining of the function "foo1"
;;      6. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the function "foo1"
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind !no_barrier_path !1 {
  %y = xor i32 %x, %x
  call void @foo(i32 %x)
  ret void
; CHECK: @dummy_barrier.
; CHECK: %y = xor i32 %x, %x
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: call void @foo(i32 %x)
; CHECK: @dummy_barrier.
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
  call void @foo1(i32 %x)
  ret void
; CHECK: call void @dummy_barrier.()
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: call void @foo1(i32 %x)
; CHECK: call void @dummy_barrier.()
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: ret void
}

; CHECK: @foo1
define void @foo1(i32 %x) nounwind {
  %y = xor i32 %x, %x
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK: @dummy_barrier.()
; CHECK-NEXT: %y = xor i32 %x, %x
; CHECK: @_Z18work_group_barrierj(i32 2)
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}


declare void @_Z18work_group_barrierj(i32)

!sycl.kernels = !{!0}
!opencl.disabled.FP_CONTRACT = !{}

!0 = !{ptr @main}
!1 = !{i1 false}

; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function main -- call void @dummy_barrier.()
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function foo -- call void @dummy_barrier.()
; DEBUGIFY-COUNT-1: WARNING: Instruction with empty DebugLoc in function foo1 -- call void @dummy_barrier.()
; DEBUGIFY-NOT: WARNING
