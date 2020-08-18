; RUN: opt -dpcpp-kernel-barrier -dpcpp-kernel-post-barrier -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;*****************************************************************************
;; This test checks the Barrier pass
;; allocates special buffer size according to local sizes:
;; dpcpp-kernel-barrier-buffer-size * LocalRange0 * LocalRange1 * LocalRange2.
;;*****************************************************************************

; CHECK: define void @main{{.*}}#[[SCAL:[0-9]+]]
define void @main(i32* %dummy0, i32* %dummy1, i32* %dummy2, i64 %LocalRange0, i64 %LocalRange1, i64 %LocalRange2, i64 %x) #0 {
; CHECK-NEXT: L1:
; CHECK-NEXT: [[SCAL1:%.*]] = mul i64 8, %LocalRange0
; CHECK-NEXT: [[SCAL2:%.*]] = mul i64 [[SCAL1]], %LocalRange1
; CHECK-NEXT: %BarrierBufferSize = mul i64 [[SCAL2]], %LocalRange2
; CHECK-NEXT: alloca i8, i64 %BarrierBufferSize, align 128
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

; CHECK: define void @__Vectorized_.main{{.*}}#[[VEC:[0-9]+]]
define void @__Vectorized_.main(i32* %dummy0, i32* %dummy1, i32* %dummy2, i64 %LocalRange0, i64 %LocalRange1, i64 %LocalRange2, i64 %x) #1 {
; CHECK-NEXT: L1:
; CHECK-NEXT: [[VEC1:%.*]] = mul i64 1, %LocalRange0
; CHECK-NEXT: [[VEC2:%.*]] = mul i64 [[VEC1]], %LocalRange1
; CHECK-NEXT: %BarrierBufferSize = mul i64 [[VEC2]], %LocalRange2
; CHECK-NEXT: alloca i8, i64 %BarrierBufferSize, align 128
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
  %y = xor i64 %x, %x
  ret void
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i64 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

; CHECK:      #[[SCAL]]{{.*}}"dpcpp-kernel-barrier-buffer-size"="8"
; CHECK-NEXT: #[[VEC]]{{.*}}"dpcpp-kernel-barrier-buffer-size"="1"

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" "vectorized_kernel"="__Vectorized_.main" }
attributes #1 = { "scalar_kernel"="main" "vectorized_width"="16" }
