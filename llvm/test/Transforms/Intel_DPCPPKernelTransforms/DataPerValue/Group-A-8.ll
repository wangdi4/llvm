; RUN: opt -disable-output 2>&1 -passes='print<dpcpp-kernel-data-per-value-analysis>' -S %s | FileCheck %s
; RUN: opt -analyze -dpcpp-kernel-data-per-value-analysis %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the DataPerValue pass
;; The case: kernel "main" with barrier instructions and the following
;;           alloca instructions (values): "%p" of size 8 bytes and 8 byte alignment
;;      0. Kernel "main" was not changed
;; The expected result:
;;  Group-A Values analysis data collected is as follows
;;      1. Data was collected only for kernel "main"
;;      2. Values "%p" were NOT collected to this group
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK: @main
define void @main(i32 %x) nounwind {
L0:
  call void @barrier_dummy()
  %p = alloca i64, align 8
  call void @barrier_dummy()
  %y = xor i32 %x, %x
  br label %L1
L1:
  call void @_Z18work_group_barrierj(i32 1)
  br label %L2
L2:
  call void @barrier_dummy()
  ret void
; CHECK: L0:
; CHECK: call void @barrier_dummy()
; CHECK: %p = alloca i64, align 8
; CHECK: call void @barrier_dummy()
; CHECK: %y = xor i32 %x, %x
; CHECK: br label %L1
; CHECK: L1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br label %L2
; CHECK: L2:
; CHECK: call void @barrier_dummy()
; CHECK: ret void
}

; CHECK: Group-A Values
; CHECK-NOT: +
; CHECK: +main
; CHECK-NOT: +
; CHECK-NOT: -
; CHECK-NOT: -p (0)


declare void @_Z18work_group_barrierj(i32)
declare void @barrier_dummy()
