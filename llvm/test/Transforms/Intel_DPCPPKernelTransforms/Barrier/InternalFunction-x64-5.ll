; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns void.
;;           kernel main also calls same "foo" function with uniform value "%x"
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. Kernel "main" stores "%x" value to offset 8 in the special buffer before calling "foo".
;;      5. Kernel "main" is still calling function "foo"
;;      6. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      7. function "foo" loads "%x" value from offset 8 in the special buffer before xor.
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i64 %x) #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i64 @__builtin_get_local_id(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i64 %y)
  br label %L2A
L2A:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  call void @foo(i64 %x)
  br label %L3A
L3A:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: xor
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: L2:                                               ; preds = %SyncBB4
; CHECK: %SBIndex2 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset3 = add nuw i64 %SBIndex2, 8
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset3
; CHECK: %pSB_LocalId4 = bitcast i8* %1 to i64*
; CHECK: %SBIndex8 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset9 = add nuw i64 %SBIndex8, 0
; CHECK: %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset9
; CHECK: %pSB_LocalId10 = bitcast i8* %2 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId10
; CHECK: store i64 %loadedValue, i64* %pSB_LocalId4
; CHECK: br label %CallBB1
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: L3:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %11 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %11 to i64*
; CHECK: store i64 %x, i64* %pSB_LocalId
; CHECK: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call void @foo
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

; CHECK-LABEL: define void @foo
define void @foo(i64 %x) #1 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %0 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId
; CHECK: %y = xor i64 %loadedValue, %loadedValue
; CHECK: br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i64 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "dpcpp-no-barrier-path"="false" }
