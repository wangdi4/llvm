; RUN: opt -disable-output 2>&1 -passes='print<dpcpp-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with 2 barier instructions before and after a call to function "foo",
;;           which returns i32 and receives non-uniform value "%y", where the return
;;           value is crossing a barrier (i.e. won't be saved in special buffer)
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected only for function "main"
;;      3. value "%y" was collected to this group
;;      4. "%y" value has offset 0 in the special buffer
;;      5. value "%ret" was collected to this group
;;      6. "%ret" value has offset 4 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      7. No analysis data was collected to this group
;;  Buffer Total Size is 16
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  %ret = call i32 @foo(i32 %y)
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  %z = add i32 %ret, %x
  br label %L3
L3:
  call void @dummy_barrier.()
  ret void
; CHECK: L0:
; CHECK: %lid = call i32 @_Z12get_local_idj(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %ret = call i32 @foo(i32 %y)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %z = add i32 %ret, %x
; CHECK: br label %L3
; CHECK: L3:
; CHECK: call void @dummy_barrier.()
; CHECK: ret void
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L3:
  %y = xor i32 %x, %x
  br label %L4
L4:
  call void @_Z18work_group_barrierj(i32 1)
  ret i32 %y
; CHECK: L3:
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L4
; CHECK: L4:
; CHECK: @_Z18work_group_barrierj(i32 1)
; CHECK: ret i32 %y
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -y (0)
; CHECK: -ret (4)
; CHECK-NOT: -
; CHECK-NOT: +
; CHECK: *

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Function Equivalence Classes:
; CHECK-NEXT: [main]: main foo

; CHECK-NEXT: Buffer Total Size:
; CHECK-NEXT: leader(main) : (16)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()
declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32)* @main}
