; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with no barier instruction that calls function "foo",
;;           which returns i32 and receives uniform value "%x", where the return
;;           value is not crossing a barrier (i.e. won't be saved in special buffer)
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. No analysis data was collected to this group
;;  Group-B.2 Values analysis data collected is as follows
;;      3. No analysis data was collected to this group
;;  Buffer Total Size is 0
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %y = xor i32 %x, %x
  %ret = call i32 @foo(i32 %x)
  ret void
; CHECK: L0:
; CHECK: %y = xor i32 %x, %x
; CHECK: %ret = call i32 @foo(i32 %x)
; CHECK: ret void
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L1:
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  ret i32 %y
; CHECK: L1:
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L2
; CHECK: L2:
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret i32 %y
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +main
; CHECK-NOT: +foo

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main foo

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (0)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()

!sycl.kernels = !{!0}

!0 = !{ptr @main}
