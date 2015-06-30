; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-ValueAnalysis  -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and values "%b", "%c" calculated using
;;            argument "%a" is crossing barrier.
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected for kernel "main"
;;      3. Only value "%y" was collected for "main"
;;      4. "%y" value has offset 0 in the special buffer
;;      5. Data was collected for function "foo"
;;      6. Only value "%b" was collected for "main"
;;      7. "%b" value has offset 8 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      8. No analysis data was collected to this group
;;  Buffer Total Size is 24
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK: @main
define void @main(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  %z = call i64 @foo(i64 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
}

; CHECK: @foo
define i64 @foo(i64 %a) nounwind {
L1:
  call void @dummybarrier.()
  %b = xor i64 %a, %a
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  %c = xor i64 %a, %b
  ret i64 %a
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
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +
; CHECK: +foo
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -b (8)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Buffer Total Size:
; CHECK-NOT: entry
; CHECK: entry(1) : (24)
; CHECK-NOT: entry
; CHECK: DONE

declare void @_Z7barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = !{void (i64)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
