; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-ValueAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with no barier instruction that calls function "foo",
;;           which returns void and receives uniform value "%x"
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. No analysis data was collected to this group
;;  Group-B.2 Values analysis data collected is as follows
;;      3. No analysis data was collected to this group
;;  Buffer Total Size is 0
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %y = xor i32 %x, %x
  call void @foo(i32 %x)
  ret void
; CHECK: L0:
; CHECK: %y = xor i32 %x, %x
; CHECK: call void @foo(i32 %x)
; CHECK: ret void
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L1:
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  ret void
; CHECK: L1:
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L2
; CHECK: L2:
; CHECK: @_Z7barrierj(i32 1)
; CHECK: ret void
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

; CHECK: Buffer Total Size:
; CHECK-NOT: entry
; CHECK: entry(0) : (0)
; CHECK-NOT: entry
; CHECK: DONE

declare void @_Z7barrierj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
