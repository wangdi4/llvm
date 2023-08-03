; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with no sync instruction but with alloca instruction
;;      0. Kernel "main" was not changed
;; The expected result:
;;      1. No analysis data was collected
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %p = alloca i64, align 4
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L1
L1:
  %z = add i32 %y, %x
  br label %L2
L2:
  ret void
; CHECK: L0:
; CHECK: %p = alloca i64, align 4
; CHECK: %lid = call i32 @_Z12get_local_idj(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: %z = add i32 %y, %x
; CHECK: br label %L2
; CHECK: L2:
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK: +main
; CHECK: -p
; CHECK: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (8)
; CHECK-NEXT: DONE

declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @main}
