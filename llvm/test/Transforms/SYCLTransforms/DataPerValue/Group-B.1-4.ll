; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following values
;;           alloca instruction "%a" of size [4 x float] (16 bytes) and alignment 4 bytes
;;           gep instruction "%p" on "%a" that does cross barrier
;;           load instruction "%x" from "%p" that does not cross barrier
;;      0. Kernel "main" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. Data was collected only for kernel "main"
;;      2. Only value "%a" collected to this group with offset 4 in the special buffer
;;  Group-B.1 Values analysis data collected is as follows
;;      3. Data was collected only for kernel "main"
;;      4. Only value "%p" collected to this group with offset 0 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      5. No analysis data was collected to this group
;;  Buffer Total Size is 20
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %arg) nounwind {
  %a = alloca [4 x float], align 4
  %p = getelementptr [4 x float], ptr %a, i32 0, i32 0
  %x = load float, ptr %p, align 4
  br label %L1

L1:                                               ; preds = %0
  call void @_Z18work_group_barrierj(i32 1)
  %y = load float, ptr %p, align 4
  ret void

; CHECK: %a = alloca [4 x float], align 4
; CHECK: %p = getelementptr [4 x float], ptr %a, i32 0, i32 0
; CHECK: %x = load float, ptr %p, align 4
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %y = load float, ptr %p, align 4
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -a (4)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -p (0)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (20)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()
declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32)* @main}
