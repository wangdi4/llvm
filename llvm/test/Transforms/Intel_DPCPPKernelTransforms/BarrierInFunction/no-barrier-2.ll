; RUN: opt -passes=dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass
;; The case: kernel "main" with no barrier instruction,
;;    which is calling function "foo" with no barrier instruction as well
;; The expected result:
;;      1. A call to @barrier_dummy() at the begining of the kernel "main"
;;      2. A call to @_Z18work_group_barrierj(LOCAL_MEM_FENCE) at the end of the kernel "main"
;;      3. No calls to @barrier_dummy or @_Z18work_group_barrierj in the function "foo"
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
  %y = xor i32 %x, %x
  call void @foo(i32 %x)
  ret void
; CHECK: @barrier_dummy()
; CHECK: %y = xor i32 %x, %x
; CHECK: call void @foo(i32 %x)
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i32 %x) #1 {
  %y = xor i32 %x, %x
  ret void
; CHECK-NOT: @barrier_dummy
; CHECK: %y = xor i32 %x, %x
; CHECK-NOT: @_Z18work_group_barrierj
; CHECK: ret
}

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "dpcpp-no-barrier-path"="false" }
