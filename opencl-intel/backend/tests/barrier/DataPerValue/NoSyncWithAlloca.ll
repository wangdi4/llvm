; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-ValueAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerValue pass
;; The case: kernel "main" with no sync instruction but with alloca instruction
;;      0. Kernel "main" was not changed
;; The expected result:
;;      1. No analysis data was collected
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %p = alloca i64, align 4
  %lid = call i32 @get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L1
L1:
  %z = add i32 %y, %x
  br label %L2
L2:
  ret void
; CHECK: L0:
; CHECK: %p = alloca i64, align 4
; CHECK: %lid = call i32 @get_local_id(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: %z = add i32 %y, %x
; CHECK: br label %L2
; CHECK: L2:
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

; CHECK: Buffer Total Size: 0

declare i32 @get_local_id(i32)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
