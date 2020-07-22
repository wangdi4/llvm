; RUN: opt -dpcpp-kernel-barrier -dpcpp-kernel-post-barrier -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;*****************************************************************************
; This test checks the Barrier pass uses passed local sizes as loop boundaries
;;*****************************************************************************

; CHECK-LABEL: define void @main
define void @main(i32* %dummy0, i32* %dummy1, i32* %dummy2, i64 %LocalRange0, i64 %LocalRange1, i64 %LocalRange2, i64 %x) #0 {
; CHECK: [[LCL_ID_SCAL_PTR0:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 0
; CHECK: [[LCL_ID_SCAL_PTR1:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 1
; CHECK: [[LCL_ID_SCAL_PTR2:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 2

; CHECK:      [[LCL_ID_SCAL_XOR:%.*]] = load i64, i64* [[LCL_ID_SCAL_PTR0]]
; CHECK-NEXT: xor i64 %x, [[LCL_ID_SCAL_XOR]]

; CHECK:      [[LCL_ID_SCAL0:%.*]] = load i64, i64* [[LCL_ID_SCAL_PTR0]]
; CHECK-NEXT: [[LCL_ID_SCAL0_INC:%.*]] = add nuw i64 [[LCL_ID_SCAL0]], 1
; CHECK:      icmp ult i64 [[LCL_ID_SCAL0_INC]], %LocalRange0

; CHECK:      [[LCL_ID_SCAL1:%.*]] = load i64, i64* [[LCL_ID_SCAL_PTR1]]
; CHECK-NEXT: [[LCL_ID_SCAL1_INC:%.*]] = add nuw i64 [[LCL_ID_SCAL1]], 1
; CHECK:      icmp ult i64 [[LCL_ID_SCAL1_INC]], %LocalRange1

; CHECK:      [[LCL_ID_SCAL2:%.*]] = load i64, i64* [[LCL_ID_SCAL_PTR2]]
; CHECK-NEXT: [[LCL_ID_SCAL2_INC:%.*]] = add nuw i64 [[LCL_ID_SCAL2]], 1
; CHECK:      icmp ult i64 [[LCL_ID_SCAL2_INC]], %LocalRange2
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

; CHECK-LABEL: define void @__Vectorized_.main
define void @__Vectorized_.main(i32* %dummy0, i32* %dummy1, i32* %dummy2, i64 %LocalRange0, i64 %LocalRange1, i64 %LocalRange2, i64 %x) #1 {
; CHECK: [[LCL_ID_VEC_PTR0:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 0
; CHECK: [[LCL_ID_VEC_PTR1:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 1
; CHECK: [[LCL_ID_VEC_PTR2:%.*]] = getelementptr inbounds [3 x i64], [3 x i64]* %pLocalIds, i64 0, i32 2

; CHECK:      [[LCL_ID_VEC_XOR:%.*]] = load i64, i64* [[LCL_ID_SCAL_PTR0]]
; CHECK-NEXT: xor i64 %x, [[LCL_ID_VEC_XOR]]

; CHECK:      [[LCL_ID_VEC0:%.*]] = load i64, i64* [[LCL_ID_VEC_PTR0]]
; CHECK-NEXT: [[LCL_ID_VEC0_INC:%.*]] = add nuw i64 [[LCL_ID_VEC0]], 16
; CHECK:      icmp ult i64 [[LCL_ID_VEC0_INC]], %LocalRange0

; CHECK:      [[LCL_ID_VEC1:%.*]] = load i64, i64* [[LCL_ID_VEC_PTR1]]
; CHECK-NEXT: [[LCL_ID_VEC1_INC:%.*]] = add nuw i64 [[LCL_ID_VEC1]], 1
; CHECK:      icmp ult i64 [[LCL_ID_VEC1_INC]], %LocalRange1

; CHECK:      [[LCL_ID_VEC2:%.*]] = load i64, i64* [[LCL_ID_VEC_PTR2]]
; CHECK-NEXT: [[LCL_ID_VEC2_INC:%.*]] = add nuw i64 [[LCL_ID_VEC2]], 1
; CHECK:      icmp ult i64 [[LCL_ID_VEC2_INC]], %LocalRange2
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

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" "vectorized_kernel"="__Vectorized_.main" }
attributes #1 = { "scalar_kernel"="main" "vectorized_width"="16" }
