; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           alloca instructions (values): "%p" of size 1 bite and 1 byte aligmnet
;;      0. Kernel "main" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. Data was collected only for kernel "main"
;;      2. Only values "%p" was collected to this group
;;      3. "%p" value has offset 0 in the special buffer
;;  Group-B.1 Values analysis data collected is as follows
;;      4. No analysis data was collected to this group
;;  Group-B.2 Values analysis data collected is as follows
;;      5. No analysis data was collected to this group
;;  Buffer Total Size is 4
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %p = alloca i1, align 1
  %y = xor i32 %x, %x
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  br label %L2
L2:
  call void @dummy_barrier.()
  ret void
; CHECK: L0:
; CHECK: %p = alloca i1, align 1
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummy_barrier.()
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -p (0)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

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
; CHECK-NEXT: leader(main) : (4)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()

!sycl.kernels = !{!0}

!0 = !{ptr @main}
