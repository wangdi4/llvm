; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -B-ValueAnalysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i64 value.
;;           kernel main also calls same "foo" function with non-uniform returned value value "%r1"
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. No analysis data was collected to this group
;;  Group-B.1 Values analysis data collected is as follows
;;      2. Data was collected only for function "main"
;;      3. value "%y" was the value collected to this group
;;      4. "%y" value has offset 0 in the special buffer
;;      5. value "%r1" was the only value collected to this group
;;      6. "%r1" value has offset 8 in the special buffer
;;  Group-B.2 Values analysis data collected is as follows
;;      7. No analysis data was collected to this group
;;  Buffer Total Size is 32
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
  %r1 = call i64 @foo(i64 %y)
  br label %L2A
L2A:
  call void @dummybarrier.()
  br label %L3
L3:
  call void @_Z7barrierj(i32 1)
  %r2 = call i64 @foo(i64 %r1)
  br label %L3B
L3B:
  call void @dummybarrier.()
  ret void
; CHECK: L1:
; CHECK:   call void @dummybarrier.()
; CHECK:   %lid = call i64 @_Z12get_local_idj(i32 0)
; CHECK:   %y = xor i64 %x, %lid
; CHECK: br label %L2
; CHECK: L2:
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   %r1 = call i64 @foo(i64 %y)
; CHECK:   br label %L2A
; CHECK: L2A:
; CHECK:   call void @dummybarrier.()
; CHECK:   br label %L3
; CHECK: L3:
; CHECK:   call void @_Z7barrierj(i32 1)
; CHECK:   %r2 = call i64 @foo(i64 %r1)
; CHECK:   br label %L3B
; CHECK: L3B:
; CHECK:   call void @dummybarrier.()
; CHECK:   ret void
}

; CHECK: @foo
define i64 @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret i64 %y
; CHECK: L1:
; CHECK:   call void @dummybarrier.()
; CHECK:   %y = xor i64 %x, %x
; CHECK:   br label %L2
; CHECK: L2:
; CHECK:   call void @_Z7barrierj(i32 2)
; CHECK:   ret i64 %y
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *
; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: -
; CHECK: -y      (0)
; CHECK: -r1     (8)
; CHECK-NOT: -
; CHECK: *
; CHECK-NOT: +
; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *
; CHECK: Buffer Total Size:
; CHECK: entry(1) : (32)
; CHECK: DONE

declare void @_Z7barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i64)* @main, metadata !1, metadata !1, metadata !"", metadata !"int", metadata !"opencl_main_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 0}
!3 = metadata !{i32 3}
!4 = metadata !{metadata !"int"}
!5 = metadata !{metadata !"x"}
