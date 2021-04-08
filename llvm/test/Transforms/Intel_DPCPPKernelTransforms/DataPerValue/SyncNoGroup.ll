; RUN: opt -analyze -dpcpp-kernel-data-per-value-analysis %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with sync instructions
;;           but without alloca instruction or any cross barrier instructions
;;      0. Kernel "main" was not changed
;; The expected result:
;;      1. No analysis data was collected
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  call void @barrier_dummy()
  %y = xor i32 %x, %x
  br label %L1
L1:
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  ret void
; CHECK: L0:
; CHECK: call void @barrier_dummy()
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L1
; CHECK: L1:
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Buffer Total Size:
; CHECK-NOT: entry
; CHECK: entry(0) : (0)
; CHECK-NOT: entry
; CHECK: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @barrier_dummy()
