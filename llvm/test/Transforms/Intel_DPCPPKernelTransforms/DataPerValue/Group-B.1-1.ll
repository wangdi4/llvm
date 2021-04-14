; RUN: opt -analyze -dpcpp-kernel-data-per-value-analysis %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           non-uniform values that cross the barrier instruction:
;;           "%y" of size 4 bytes, while "%z" of size 8 bytes.
;;      0. Kernel "main" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected only for kernel "main"
;;      3. Only values "%y" and "%z" was collected to this group
;;      4. "%y" value has offset 0 in the special buffer
;;      5. "%z" value has offset 8 in the special buffer (because of the alignment needed by "%z")
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
  %z = zext i32 %y to i64
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  %w = trunc i64 %z to i32
  %t = add i32 %y, %w
  br label %L2
L2:
  call void @barrier_dummy()
  ret void
; CHECK: L0:
; CHECK: %lid = call i32 @_Z12get_local_idj(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: %z = zext i32 %y to i64
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %w = trunc i64 %z to i32
; CHECK: %t = add i32 %y, %w
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @barrier_dummy()
; CHECK: ret void
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
; CHECK: -z (8)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Buffer Total Size:
; CHECK-NOT: entry
; CHECK: entry(0) : (16)
; CHECK-NOT: entry
; CHECK: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @barrier_dummy()
declare i32 @_Z12get_local_idj(i32)
