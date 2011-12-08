; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-FunctionAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with no barier instruction that calls function "foo",
;;           which returns void and receives uniform value "%x"
;; The expected result:
;;      0. Kernel "main" and function "foo" were not changed
;;  Data collected on functions analysis data collected as follow
;;      1. Data was collected only for function "foo" with the following outputs
;;         a. need to be fixed: 0
;;         b. number of usages: 1
;;         c. In special buffer counters: ( 0 )
;;  Data collected on calls analysis data collected as follow
;;      2. Data was collected only for "call void @foo(i32 %x)" with the following outputs
;;         a. Offsets in special buffer: ( BAD_OFFSET )
;;  Ordered functions to fix analysis data collected as follow
;;      3. No data collected for "foo" or "main"
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

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
  call void @barrier(i32 1)
  ret void
; CHECK: L1:
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L2
; CHECK: L2:
; CHECK: @barrier(i32 1)
; CHECK: ret void
}

; CHECK: Data collected on functions
; CHECK-NOT: main
; CHECK: foo
; CHECK: need to be fixed: 0     number of usages: 1     In special buffer counters: ( 0 )
; CHECK-NOT: main

; CHECK: Data collected on calls
; CHECK: call void @foo(i32 %x)
; CHECK: Offsets in special buffer: ( BAD_OFFSET )

; CHECK: Ordered functions to fix
; CHECK-NOT: main
; CHECK-NOT: foo

; CHECK: DONE

declare void @barrier(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
