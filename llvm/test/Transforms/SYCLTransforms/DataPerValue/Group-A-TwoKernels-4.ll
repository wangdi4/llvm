; RUN: opt -disable-output 2>&1 -passes='print<sycl-kernel-data-per-value-analysis>' -S < %s | FileCheck %s

;;*****************************************************************************
; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           alloca instructions (values):
;;           "%p1" of size 8 bytes and 16 byte aligmnet,
;;           while "%p2" of size 10x8 bytes and 8 byte aligmnet.
;;           kernel "foo" with barrier instructions and the following
;;           alloca instructions (values):
;;           "%p1" of size 8 bytes and 16 byte aligmnet
;;           function "bar" that is caled from both kernels
;;      0. Kernels "main", "foo" and function "bar" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. Data was collected for kernel "foo"
;;      2. Only values "%p1" and "%p2" were collected to this group
;;      3. "%p1" value has offset 0 in the special buffer
;;  Group-B.1 Values analysis data collected is as follows
;;      4. No analysis data was collected to this group
;;  Group-B.2 Values analysis data collected is as follows
;;      5. No analysis data was collected to this group
;;  Buffer Total Size is 112 (because of the alignment needed by "%p1", i.e. 16 bytes)
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  %p1 = alloca i64, align 16
  %p2 = alloca [10 x i64], align 8
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  call void @bar(i32 0)
  br label %L2
L2:
  call void @dummy_barrier.()
  ret void
; CHECK: L0:
; CHECK: %p1 = alloca i64, align 16
; CHECK: %p2 = alloca [10 x i64], align 8
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: call void @bar(i32 0)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummy_barrier.()
; CHECK: ret void
}

; CHECK: @foo
define void @foo(i32 %x) nounwind {
L0:
  %p1 = alloca i64, align 16
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  call void @bar(i32 0)
  br label %L2
L2:
  call void @dummy_barrier.()
  ret void
; CHECK: L0:
; CHECK: %p1 = alloca i64, align 16
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: call void @bar(i32 0)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @dummy_barrier.()
; CHECK: ret void
}

; CHECK: @bar
define void @bar(i32 %x) nounwind {
L0:
  ret void
; CHECK: L0:
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

; CHECK: Function Equivalence Classes:
; CHECK-DAG: [main]: main
; CHECK-DAG: [foo]: foo
; CHECK-DAG: [bar]: bar

; CHECK-NEXT: Buffer Total Size:
; CHECK-DAG: leader(main) : (96)
; CHECK-DAG: leader(foo) : (16)
; CHECK-DAG: leader(bar) : (0)
; CHECK-NEXT: DONE

declare void @_Z18work_group_barrierj(i32)
declare void @dummy_barrier.()
declare i32 @_Z12get_local_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @main}
