; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-FunctionAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the DataPerInternalFunction pass
;; The case: kernel "main" with barier instruction that calls function "foo",
;;           which returns void and receives alloca value "%p"
;; The expected result:
;;      0. Kernel "main" and function "foo" were not changed
;;  Data collected on functions analysis data collected as follow
;;      1. Data was collected for function "foo" with the following outputs
;;         a. need to be fixed: 1
;;         b. number of usages: 1
;;         c. In special buffer counters: ( 1 )
;;  Data collected on calls analysis data collected as follow
;;      2. Data was collected only for "call void @foo(i32 %p)" with the following outputs
;;         a. Offsets in special buffer: ( 4 )
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
  %p = alloca i32, align 4
  %lid = call i32 @get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L1
L1:
  call void @barrier(i32 1)
  store i32 %y, i32* %p, align 4
  call void @foo(i32* %p)
  br label %L2
L2:
  call void @dummybarrier.()
  ret void
; CHECK: L0:
; CHECK: %p = alloca i32, align 4
; CHECK: %lid = call i32 @get_local_id(i32 0)
; CHECK: %y = xor i32 %x, %lid
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @barrier(i32 1)
; CHECK: store i32 %y, i32* %p, align 4
; CHECK: call void @foo(i32* %p)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummybarrier.()
; CHECK: ret void
}

; CHECK: @foo
define void @foo(i32* %x) nounwind {
L3:
  %z = load i32* %x, align 4
  %y = xor i32 %z, %z
  br label %L4
L4:
  call void @barrier(i32 1)
  ret void
; CHECK: L3:
; CHECK: %z = load i32* %x, align 4
; CHECK: %y = xor i32 %z, %z
; CHECK: br label %L4
; CHECK: L4:
; CHECK: @barrier(i32 1)
; CHECK: ret void
}

; CHECK: Data collected on functions
; CHECK: foo
; CHECK: need to be fixed: 1     number of usages: 1     In special buffer counters: ( 1 )

; CHECK: Data collected on calls
; CHECK: call void @foo(i32* %p)
; CHECK: Offsets in special buffer: ( 4 )

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
