; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and function "foo",
;;           which contains get_local_id() and returns void.
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" is still calling function "foo"
;;      3. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      4. function "foo" calls get_curr_wi exactly once
;;      5. function "foo" calls get_new_local_id.() instead of get_local_id
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"

; CHECK-LABEL: define void @main
define void @main() #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i32 0)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: call void @foo(i32 0, [3 x i32]* %pLocalIds)
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

define void @foo(i32 %x) {
; CHECK-LABEL: define void @foo(i32 %x, [3 x i32]* noalias %pLocalIdValues)
L1:
  call i32 @__builtin_get_local_id(i32 0)
  br label %L2
L2:
  ret void

; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: %pLocalId_0 = getelementptr inbounds [3 x i32], [3 x i32]* %pLocalIdValues, i32 0, i32 0
; CHECK: %LocalId_0 = load i32, i32* %pLocalId_0
; CHECK: br label %L2
; CHECK: L2:                                               ; preds = %L1
; CHECK: ret
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()
declare i32 @__builtin_get_local_id(i32)

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
