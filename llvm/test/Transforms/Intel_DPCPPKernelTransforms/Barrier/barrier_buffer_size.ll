; RUN: opt -passes=dpcpp-kernel-barrier %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

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

define void @main(i64 %x) #0 !no_barrier_path !{i1 0} !vectorized_kernel !{void (i64)* @__Vectorized_.main} {
; CHECK: define void @main
; CHECK-SAME: !barrier_buffer_size ![[SCAL:[0-9]+]]
L1:
  call void @barrier_dummy()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @barrier_dummy()
  ret void
}

define void @__Vectorized_.main(i64 %x) #1 !vectorized_width !{i32 16} {
; CHECK: define void @__Vectorized_.main
; CHECK-SAME: !barrier_buffer_size ![[VEC:[0-9]+]]
L1:
  call void @barrier_dummy()
  %lid = call i64 @_Z12get_local_idj(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @barrier_dummy()
  ret void
}

define void @foo(i64 %x) {
L1:
  call void @barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @_Z18work_group_barrierj(i32 2)
  ret void
}

declare void @_Z18work_group_barrierj(i32)
declare i64 @_Z12get_local_idj(i32)
declare void @barrier_dummy()

; CHECK-DAG: ![[SCAL]] = !{i32 24}
; CHECK-DAG: ![[VEC]] = !{i32 2}

attributes #0 = { "no-barrier-path"="false" "sycl-kernel" "vectorized-kernel"="__Vectorized_.main" }
attributes #1 = { "vectorized-width"="16" }


!sycl.kernels = !{!0}
!0 = !{void (i64)* @main}
