; RUN: %oclopt -analyze -B-ValueAnalysis -verify -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           alloca instructions (values):
;;           "%p1" of size 8 bytes and 16 byte aligmnet,
;;           while "%p2" of size 10x8 bytes and 8 byte aligmnet.
;;           kernel "foo" with barrier instructions and the following
;;           alloca instructions (values):
;;           "%p1" of size 8 bytes and 16 byte aligmnet
;;      0. Kernels "main" and "foo" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. Data was collected for kernel "foo"
;;      2. Only values "%p1" and "%p2" were collected to this group
;;      3. "%p1" value has offset 0 in the special buffer
;;  Group-B.1 Values analysis data collected is as follows
;;      4. No analysis data was collected to this group
;;  Group-B.2 Values analysis data collected is as follows
;;      5. No analysis data was collected to this group
;;  Buffer Total Size for main is 96 (because of the alignment needed by "%p1", i.e. 16 bytes)
;;  Buffer Total Size for foo is 16 (because of the alignment needed by "%p1", i.e. 16 bytes)
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %p1 = alloca i64, align 16
  %p2 = alloca [10 x i64], align 8
  br label %L1
L1:
  call void @_Z7barrierj(i32 1)
  br label %L2
L2:
  call void @dummybarrier.()
  ret void
; CHECK: L0:
; CHECK: %p1 = alloca i64, align 16
; CHECK: %p2 = alloca [10 x i64], align 8
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummybarrier.()
; CHECK: ret void
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L0:
  %p1 = alloca i64, align 16
  br label %L1
L1:
  call void @_Z7barrierj(i32 1)
  br label %L2
L2:
  call void @dummybarrier.()
  ret void
; CHECK: L0:
; CHECK: %p1 = alloca i64, align 16
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummybarrier.()
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK: +foo
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK: -p1 (0)
; CHECK-NOT: -
; CHECK: *

; CHECK: Group-B.1 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Group-B.2 Values
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: *

; CHECK: Buffer Total Size:
; CHECK-NOT: entry
; CHECK: entry(0) : (96)
; CHECK: entry(1) : (16)
; CHECK-NOT: entry
; CHECK: DONE

declare void @_Z7barrierj(i32)
declare void @dummybarrier.()
declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32)* @main}
