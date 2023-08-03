; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-barrier-wi-analysis>' %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and values "%b", "%c" calculated using
;;            argument "%a" is crossing barrier.
;; The expected result:
;;      1. "%lid" is non-uniform value (i.e. WI related)
;;      2. "%y" is non-uniform value (i.e. WI related)
;;      3. "%z" is non-uniform value (i.e. WI related)
;;      4. "%b" is non-uniform value (i.e. WI related)
;;      5. "%c" is non-uniform value (i.e. WI related)
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; CHECK-LABEL: @main
define void @main(i64 %x) {
L1:
  call void @dummy_barrier.()
  %lid = call i64 @_Z12get_local_idj(i64 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  %z = call i64 @foo(i64 %y)
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
}

; CHECK-LABEL: @foo
define i64 @foo(i64 %a) nounwind {
L1:
  call void @dummy_barrier.()
  %b = xor i64 %a, %a
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  %c = xor i64 %a, %b
  ret i64 %a
}


; CHECK: WI related Values
; CHECK: %lid is WI related
; CHECK: %y is WI related
; CHECK: %z is WI related
; CHECK: %b is WI related
; CHECK: %c is WI related

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i64)
declare void @dummy_barrier.()

!sycl.kernels = !{!0}
!0 = !{ptr @main}
