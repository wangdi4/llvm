; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-ValueAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           non-uniform values that cross the barrier instruction:
;;           "%z" of size 8 bytes, while "%y" and "%v" of size 4 bytes each.
;;      0. Kernel "main" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected only for kernel "main"
;;      3. Only values "%z", "%y" and "%v" was collected to this group
;;      4. "%z" value has offset 0 in the special buffer
;;      5. "%y" value has offset 8 in the special buffer
;;      6. "%v" value has offset 12 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      7. No analysis data was collected to this group
;;  Buffer Total Size is 16
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %lid = call i32 @get_local_id(i32 0)
  %z = zext i32 %lid to i64
  %y = xor i32 %x, %lid
  %v = add i32 %y, %lid
  br label %L1
L1:
  call void @barrier(i32 1)
  %w = trunc i64 %z to i32
  %t = add i32 %y, %v
  br label %L2
L2:
  call void @dummybarrier.()
  ret void
; CHECK: L0:
; CHECK: %lid = call i32 @get_local_id(i32 0)
; CHECK: %z = zext i32 %lid to i64
; CHECK: %y = xor i32 %x, %lid
; CHECK: %v = add i32 %y, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @barrier(i32 1)
; CHECK: %w = trunc i64 %z to i32
; CHECK: %t = add i32 %y, %v
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummybarrier.()
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
; CHECK: -z (0)
; CHECK: -y (8)
; CHECK: -v (12)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Buffer Total Size: 16

declare void @barrier(i32)
declare void @dummybarrier.()
declare i32 @get_local_id(i32)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
