; RUN: opt -passes=dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass
;; The case: kernel "main" with no barrier instruction,
;;    which is calling function "foo" that contains barrier instruction
;; The expected result:
;;      1. A call to @barrier_dummy() at the begining of the kernel "main"
;;      2. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) just before calling the function "foo"
;;      3. A call to @barrier_dummy() just after calling the function "foo"
;;      4. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the kernel "main"
;;      5. A call to @barrier_dummy() at the begining of the function "foo"
;;      6. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the function "foo"
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
  %y = xor i32 %x, %x
  call void @foo(i32 %x)
  ret void
; CHECK: @barrier_dummy
; CHECK: %y = xor i32 %x, %x
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: call void @foo(i32 %x)
; CHECK: @barrier_dummy
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i32 %x) {
  %y = xor i32 %x, %x
  call void @_Z18work_group_barrierj(i32 2)
  ret void
; CHECK: @barrier_dummy()
; CHECK-NEXT: %y = xor i32 %x, %x
; CHECK: @_Z18work_group_barrierj(i32 2)
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}

declare void @_Z18work_group_barrierj(i32)

attributes #0 = { "no-barrier-path"="false" "sycl-kernel" }
attributes #1 = { "no-barrier-path"="false" }
