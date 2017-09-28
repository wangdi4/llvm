; RUN: %oclopt -B-Barrier -verify -S < %s | FileCheck %s
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;*****************************************************************************
; This test checks the Barrier pass
;; sets barrier_buffer_size
;; The expected result:
;;      1. Kernel "main" has barrier_buffer_size metadata set
;;      2. Kernel "__Vectorized_.main" has barrier_buffer_size metadata set
;;*****************************************************************************

define void @main(i64 %x) nounwind !vectorized_kernel !1 {
L1:
  call void @dummybarrier.()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
}

; CHECK: @main{{.*}} !barrier_buffer_size ![[SCAL:[0-9]+]] {{.*}} {

define void @__Vectorized_.main(i64 %x) nounwind !vectorized_width !2 {
L1:
  call void @dummybarrier.()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
}

; CHECK: @__Vectorized_.main{{.*}} !barrier_buffer_size ![[VEC:[0-9]+]] {{.*}} {

define void @foo(i64 %x) nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
  ret void
}

declare void @_Z7barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

!opencl.kernels = !{!0}

!0 = !{void (i64)* @main}
!1 = !{void (i64)* @__Vectorized_.main}
!2 = !{i32 16}

; CHECK-DAG: ![[SCAL]] = !{i32 {{[0-9]+}}}
; CHECK-DAG: ![[VEC]] = !{i32 {{[0-9]+}}}
