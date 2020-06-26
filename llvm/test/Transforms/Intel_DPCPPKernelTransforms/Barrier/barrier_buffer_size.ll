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

define void @main(i64 %x) #0 {
; CHECK: @main{{.*}}#[[SCAL:[0-9]+]]
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i64 @__builtin_get_local_id(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
}

define void @__Vectorized_.main(i64 %x) #1 {
; CHECK: @__Vectorized_.main{{.*}}#[[VEC:[0-9]+]]
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i64 @__builtin_get_local_id(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i64 %y)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
}

define void @foo(i64 %x) {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret void
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i64 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

; CHECK:      #[[SCAL]]{{.*}}"dpcpp-kernel-barrier-buffer-size"="24"
; CHECK-NEXT: #[[VEC]]{{.*}}"dpcpp-kernel-barrier-buffer-size"="2"

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" "vectorized_kernel"="__Vectorized_.main" }
attributes #1 = { "vectorized_width"="16" }

