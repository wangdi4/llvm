; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-FunctionAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with 2 barier instructions before and after a call to function "foo",
;;           which returns i32 and receives non-uniform value "%y", where the return 
;;           value is crossing a barrier (i.e. won't be saved in special buffer)
;; The expected result:
;;      0. Kernel "main" and function "foo" were not changed
;;  Data collected on functions analysis data collected as follow
;;      1. Data was collected only for function "foo" with the following outputs
;;         a. need to be fixed: 1
;;         b. number of usages: 1
;;         c. In special buffer counters: ( 1, 1 )
;;  Data collected on calls analysis data collected as follow
;;      2. Data was collected only for "call void @foo(i32 %y)" with the following outputs
;;         a. Offsets in special buffer: ( 0, 4 ) //TODO: can we assume this order of offsets?
;;  Ordered functions to fix analysis data collected as follow
;;      3. Data collected only for "foo"
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
2:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %lid = call i32 @get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L1
L1:
  call void @barrier(i32 1)
  %ret = call i32 @foo(i32 %y)
  br label %L2
L2:
  call void @barrier(i32 1)
  %z = add i32 %ret, %x
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
; CHECK: L0:
; CHECK: %lid = call i32 @get_local_id(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @barrier(i32 1)
; CHECK: %ret = call i32 @foo(i32 %y)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @barrier(i32 1)
; CHECK: %z = add i32 %ret, %x
; CHECK: br label %L3
; CHECK: L3:
; CHECK: call void @dummybarrier.()
; CHECK: ret void
}

; CHECK: @foo
define i32 @foo(i32 %x) nounwind {
L3:
  %y = xor i32 %x, %x
  br label %L4
L4:
  call void @barrier(i32 1)
  ret i32 %y
; CHECK: L3:
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L4
; CHECK: L4:
; CHECK: @barrier(i32 1)
; CHECK: ret i32 %y
}

; CHECK: Data collected on functions
; CHECK: foo
; CHECK: need to be fixed: 1     number of usages: 1     In special buffer counters: ( 1, 1 )

; CHECK: Data collected on calls
; CHECK: ret = call i32 @foo(i32 %y)
; CHECK: Offsets in special buffer: ( 0, 4 )

; CHECK: Ordered functions to fix
; CHECK-NOT: main
; CHECK: foo
; CHECK-NOT: main

; CHECK: DONE

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
